
require_relative 'HTTPCore.rb'

module XNM
	module Telegram
		# This class handles translating the sometimes a bit interesting
		# Telegram API data to more usable types.
		# It also handles the translation of User-IDs to the Usernames,
		# and provides "Grouping IDs" to make it easier to edit, reply to, and
		# delete messages
		# It also exposes a much neater way of constructing inline keyboards.
		class GroupingAdapter
			attr_accessor :usernameList
			attr_reader   :groupIDList

			attr_reader :testLastUID
			attr_reader :testLastData

			def initialize(httpCore)
				# Check if we already have a HTTPCore, else create one
				@httpCore = if(httpCore.is_a? Telegram::HTTPCore)
						httpCore;
					else
						Telegram::HTTPCore.new(httpCore);
					end
				@httpCore.attach_receptor(self);

				_reset();
			end

			def _reset()
				# Hash {username => ChatID}
				@usernameList 	= Hash.new();
				# Hash {ChatID => {GroupID => MessageID}}
				@groupIDList 	= Hash.new do |hash, key|
					hash[key] = Hash.new;
				end
			end

			# Tested in ts_group_adapter/test_keyboard_build
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

			# Processes messages received through MQTT/Custom input
			# It takes care of setting a few good defaults (like parse_mode),
			# deletes any old messages of the same GroupID (if requested),
			# and stores the new Message ID for later processing
			# @param data [Hash] The message packet to be sent to Telegram
			#   The "text" field is always required. Optional fields are:
			#   - gid: Grouping-ID for later editing/deleting/replying to
			#   - replace: true/false whether or not the old GID-Tagged message should be deleted
			#   - overwrite: Similar to replace, but instead of re-sending, it only edits the old message
			#   - silent: Sets the "disable_notification" flag
			#   - inline_keyboard: Hash of inline keyboard buttons (Button-text as key, button reply as value)
			# @param uID [Integer,String] The user-id as received from the MQTT Wildcard.
			#   Can be a username defined in @usernameList, or the raw Chat ID
			# Tested in ts_mqtt/test_send
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

			# Edits an already known message. Takes arguments similar to _handle_send
			# @param data [Hash] The message to be edited. GID must be set, and optionally
			#   a inline keyboard markup or a new message text must be provided.
			# @param uID [Integer,String] The user-id as received from the MQTT Wildcard.
			#   Can be a username defined in @usernameList, or the raw Chat ID
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

			# Deletes a message marked by a given GID
			# @param gid [String] The grouping-ID of the message to delete.
			# @param uID [Integer, String] The User-ID (as defined in @usernameList, or
			#    the raw ID), for which to delete.
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

			def on_message(data, uID)
				@testLastUID  = uID;
				@testLastData = data;
			end

			def on_command(data, uID)
			end

			def on_reply(data, uID)
			end

			def on_callback_pressed(data, uID)
			end

			# Handle an incoming message packet from the HTTP core
			# @private
			def handle_message(msg)
				uID = msg[:chat][:id];
				# Resolve the User-ID, if it's known.
				if(newUID = @usernameList.key(uID))
					uID = newUID
				end

				data = Hash.new();
				# Only accept messages that contain text (things like keyboard replies
				# are handled elsewhere).
				return unless(data[:text] = msg[:text])

				# See if this message was a reply, and if we know said reply under a group-id
				if(replyMSG = msg[:reply_to_message])
					data[:reply_gid] = @groupIDList[uID].key(replyMSG[:message_id]);
				end

				# Distinguish the type of message. If it starts with a command-slash,
				# it will be excempt from normal processing.
				# If it has a reply message ID that we know, handle it as a reply.
				# Otherwise, simply send it off as a normal message.
				if(data[:text] =~ /^\//)
					on_command(data, uID)
				elsif(data[:reply_gid])
					on_reply(data, uID)
				else
					on_message(data, uID)
				end
			end

			# Handle an incoming callback query (inline keyboard button press)
			# as received from the HTTP Core
			# @private
			def handle_callback_query(cbq)
				# Send out a callback query reply (i.e. Telegram now knows we saw it)
				@httpCore.perform_post("answerCallbackQuery", {callback_query_id: cbq[:id]});

				# Resolve the username, if we know it.
				uID = msg[:message][:chat][:id];
				if(newUID = @usernameList.key(uID))
					uID = newUID
				end

				# Try to parse the data. This gem sets inline keyboard reply data to
				# a small JSON, which identifies the GID and the key that was pressed.
				begin
					data = JSON.parse(cbq[:data], symbolize_names: true);
				rescue
					return;
				end

				data = {
					gid:  data[:i],
					key: data[:k],
				}

				# If the key ID starts with a command slash, treat it like a normal
				# command. Has the benefit of making it super easy to execute already
				# implemented actions as a inline keyboard
				if(data[:key] =~ /^\//)
					on_command({text: data[:key], gid: data[:gid]}, uID);
				end
				on_callback_pressed(data, uID);
			end

			# Handle incoming HTTP Core packets. Just send them to the appropriate
			# handler function
			def handle_packet(packet)
				if(msg = packet[:message])
					handle_message(msg);
				end

				if(cbq = packet[:callback_query])
					handle_callback_query(cbq);
				end
			end
		end
	end
end
