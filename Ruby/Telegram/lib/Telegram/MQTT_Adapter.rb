
require 'mqtt/sub_handler'

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
				@groupIDList 	= Hash.new();

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
				uID = uID[0];
				uID = @usernameList[uID] if(@usernameList.key? uID)
				uID = uID.to_i;

				begin
					data = JSON.parse(data, symbolize_names: true);
				rescue
					data = {text: data}
				end

				data[:parse_mode] ||= "Markdown";
				data[:chat_id]		= uID;

				reply = @httpCore.perform_post("sendMessage", data);

				if(gID = data[:GID])
					if(data[:single] and (mID = @groupIDList[uID][gID]))
						@httpCore.perform_post("deleteMessage", {chat_id: uID, message_id: mID})
					end

					@groupIDList[uID][gID] = reply[:result][:message_id];
				end
			end

			def setup_mqtt()
				@mqtt.subscribe_to "Telegram/+/Send" do |data, uID|
					_handle_send(data, uID);
				end

				@mqtt.subscribe_to "Telegram/+/Edit" do |data, uID|
					uID = uID[0];
					uID = @usernameList[uID] if(@usernameList.key? uID)
					uID = uID.to_i;

					begin
						data = JSON.parse(data, symbolize_names: true);

						if(mID = data[:message_id])
							if(newMID = @groupIDList[uID].key(mID))
								data[:message_id] = newMID;
							end
						end

						data[:chat_id] = uID;
					rescue
					end
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
