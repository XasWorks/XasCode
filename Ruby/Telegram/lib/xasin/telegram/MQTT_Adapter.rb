
require_relative 'HTTPCore.rb'
require 'mqtt/sub_handler'

module Xasin
module Telegram
	module MQTT
		class Server
			attr_accessor :usernameList

			def initialize(httpCore, mqtt)
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
				uID = uID.to_i;

				begin
					data = JSON.parse(data, symbolize_names: true);
				rescue
					# Allow for pure-text to be sent (easier on the ESPs)
					data = {text: data}
				end

				data[:parse_mode] ||= "Markdown";
				data[:chat_id]		= uID;

				reply = @httpCore.perform_post("sendMessage", data);

				# Check if this message has a grouping ID
				if(gID = data[:GID])
					# If the message has the :single flag, delete the last one
					if(data[:single])
						_handle_delete(gID, uID);
					end

					# Save this grouping ID
					@groupIDList[uID][gID] = reply[:result][:message_id];
				end
			end

			def _handle_edit(data, uID)
				# Resolve a saved Username to a User-ID
				uID = @usernameList[uID] if(@usernameList.key? uID)
				uID = uID.to_i;

				begin
					data = JSON.parse(data, symbolize_names: true);

					return unless data[:text];
					# Fetch the target Message ID
					return unless mID = @groupIDList[uID][data[:GID]]

					# Send the POST request editing the message text
					@httpCore.perform_post("editMessageText",
							{	text: data[:text],
								chat_id: 	uID,
								message_id:	mID});
				rescue
				end
			end

			def _handle_delete(data, uID)
				# Resolve a saved Username to a User-ID
				uID = @usernameList[uID] if(@usernameList.key? uID)
				uID = uID.to_i;

				# Fetch the real message ID held by a grouping ID
				return unless mID = @groupIDList[uID][data]
				@groupIDList[uID].delete(data);

				@httpCore.perform_post("deleteMessage", {chat_id: uID, message_id: mID});
			end

			def setup_mqtt()
				@mqtt.subscribe_to "Telegram/+/Send" do |data, tSplit|
					_handle_send(data, tSplit[0]);
				end

				@mqtt.subscribe_to "Telegram/+/Edit" do |data, tSplit|
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
						data[:reply_GID] = @groupIDList[uID].key(replyMSG[:message_id]);
					end

					if(data[:reply_GID])
						@mqtt.publish_to "Telegram/#{uID}/Reply", data;
					else
						@mqtt.publish_to "Telegram/#{uID}/Received", data;
					end
				end
			end
		end
	end
end
end
