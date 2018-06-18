
require_relative 'HTTPCore.rb'
require 'mqtt/sub_handler'

module Xasin
module Telegram
	module MQTT
		class Server
			attr_accessor :usernameList

			def initialize(httpCore, mqtt)
				# Check if we already have a HTTPCore, else create one
				if(httpCore.is_a? Telegram::HTTPCore)
					@httpCore = httpCore;
				else
					@httpCore = Telegram::HTTPCore.new(httpCore);
				end
				@httpCore.attach_receptor(self);

				@mqtt = mqtt;

				# Hash {username => ChatID}
				@usernameList 	= Hash.new();
				# Hash {ChatID => {GroupID => MessageID}}
				@groupIDList 	= Hash.new do |hash, key|
					hash[key] = Hash.new;
				end

				setup_mqtt();
			end

			def _process_inline_keyboard(keyboardLayout, gID = nil)
				# Return unless we have a structure we can form into a keyboard
				return nil unless (keyboardLayout.is_a? Array or keyboardLayout.is_a? Hash)

				# Make sure the structure of keyboardLayout is [{}] or [[]]
				if(keyboardLayout.is_a? Hash)
					keyboardLayout = [keyboardLayout]
				elsif(not (keyboardLayout[0].is_a? Array or keyboardLayout[0].is_a? Hash))
					keyboardLayout = [keyboardLayout]
				end

				outData = Array.new();

				# Iterate through the rows of keyboards
				keyboardLayout.each do |row|
					newRow = Array.new();

					# Create the INLINE KEY button elements
					row.each do |key, val|
						cbd = {i: gID, k: (val or key)};
						newRow << {text: key, callback_data: cbd.to_json}
					end

					# Add the new row to the array of rows
					outData << newRow;
				end

				# Return the ready reply_markup element
				return {inline_keyboard: outData};
			end

			# Processes messages received through MQTT
			# It takes care of setting a few good defaults (like parse_mode),
			# deletes any old messages of the same GroupID (if requested),
			# and stores the new Message ID for later processing
			# @param data [Hash] The raw "message" object received from the Telegram API
			# @param uID [Integer,String] The user-id as received from the MQTT Wildcard.
			#   Can be a username defined in @usernameList
			def _handle_send(data, uID)
				# Resolve a saved Username to a User-ID
				uID = @usernameList[uID] if(@usernameList.key? uID)
				return if (uID = uID.to_i) == 0; # Return if a unknown Username was used

				gID = data[:gid];

				# Check if a GroupID is present and a former message is known
				if(gID and @groupIDList[uID][gID])
					if(data[:replace])
						_handle_delete(gID, uID)
					elsif(data[:overwrite])
						_handle_edit(data, uID);
						return; # After editing, no new message should be sent!
					end
				end

				# Lay out all mandatory parameters for sendMessage request
				outData = {
					chat_id: 	uID,
					parse_mode: (data[:parse_mode] or "Markdown"), # Markdown parse mode is just nice
					text:			data[:text]
				}

				# Check if the message is meant to be sent without notification
				if(data[:silent])
					outData[:disable_notification] = true;
				end

				# Check if an inline keyboard layout is given, and parse that.
				if((ilk = data[:inline_keyboard]))
					outData[:reply_markup] = _process_inline_keyboard(ilk, gID);
				end

				reply = @httpCore.perform_post("sendMessage", outData);
				return unless reply[:ok]	# Something was wrong about our message layout
				# TODO Add a proper error handler here.

				# If a GroupID was given, save the sent message's ID
				@groupIDList[uID][gID] = reply[:result][:message_id] if(gID);
			end

			def _handle_edit(data, uID)
				# Resolve a saved Username to a User-ID
				uID = @usernameList[uID] if(@usernameList.key? uID)
				return if (uID = uID.to_i) == 0; # Return if a unknown Username was used

				# Fetch the target MessageID - Return if none is present
				return unless mID = @groupIDList[uID][data[:gid]]

				# Lay out all mandatory arguments for the edit POST
				outData = {
					chat_id: uID,
					message_id: mID,
				};

				# If a inline keyboard was given, parse that.
				if(ilk = data[:inline_keyboard])
					outData[:reply_markup] = _process_inline_keyboard(ilk, data[:gid]);
				end

				if(data[:text]) # Check if text was given
					outData[:text] = data[:text];
					# Send the POST request editing the message text
					@httpCore.perform_post("editMessageText", outData);
				else
					# Otherwise, only edit the reply markup (keyboard etc.)
					@httpCore.perform_post("editMessageReplyMarkup", outData);
				end
			end

			def _handle_delete(data, uID)
				# Resolve a saved Username to a User-ID
				uID = @usernameList[uID] if(@usernameList.key? uID)
				return if (uID = uID.to_i) == 0; # Return unless the username was known

				# Fetch the real message ID held by a grouping ID
				return unless mID = @groupIDList[uID][data]
				@groupIDList[uID].delete(data); # Clear that ID from the list

				# Perform the actual delete
				@httpCore.perform_post("deleteMessage", {chat_id: uID, message_id: mID});
			end

			def setup_mqtt()
				@mqtt.subscribe_to "Telegram/+/Send" do |data, tSplit|
					begin
						data = JSON.parse(data, symbolize_names: true);
					rescue
						data = {text: data}
					end

					_handle_send(data, tSplit[0]);
				end

				@mqtt.subscribe_to "Telegram/+/Edit" do |data, tSplit|
					begin
						data = JSON.parse(data, symbolize_names: true);
					rescue
						next;
					end

					_handle_edit(data, tSplit[0])
				end

				@mqtt.subscribe_to "Telegram/+/Delete" do |data, tSplit|
					_handle_delete(data, tSplit[0])
				end

				@mqtt.subscribe_to "Telegram/+/Release" do |data, tSplit|
					# Resolve a saved Username to a User-ID
					uID = tSplit[0];
					uID = @usernameList[uID] if(@usernameList.key? uID)
					uID = uID.to_i;

					# Delete the stored GID key
					@groupIDList[uID].delete(data);
				end
			end

			def handle_packet(packet)
				if(msg = packet[:message])
					uID = msg[:chat][:id];
					if(newUID = @usernameList.key(uID))
						uID = newUID
					end

					data = Hash.new();
					return unless(data[:text] = msg[:text])

					if(replyMSG = msg[:reply_to_message])
						data[:reply_gid] = @groupIDList[uID].key(replyMSG[:message_id]);
					end

					if(data[:text] =~ /^\//)
						@mqtt.publish_to "Telegram/#{uID}/Command", data.to_json;
					elsif(data[:reply_gid])
						@mqtt.publish_to "Telegram/#{uID}/Reply", data.to_json;
					else
						@mqtt.publish_to "Telegram/#{uID}/Received", data.to_json;
					end
				end

				if(msg = packet[:callback_query])
					@httpCore.perform_post("answerCallbackQuery", {callback_query_id: msg[:id]});

					uID = msg[:message][:chat][:id];
					if(newUID = @usernameList.key(uID))
						uID = newUID
					end

					begin
						data = JSON.parse(msg[:data], symbolize_names: true);

						data = {
							gid:  data[:i],
							key: data[:k],
						}

						if(data[:key] =~ /^\//)
							@mqtt.publish_to "Telegram/#{uID}/Command", {text: data[:key]}.to_json
						end
						@mqtt.publish_to "Telegram/#{uID}/KeyboardPress", data.to_json
					rescue
					end
				end
			end
		end
	end
end
end
