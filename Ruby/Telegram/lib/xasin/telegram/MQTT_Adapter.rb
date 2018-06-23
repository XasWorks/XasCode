
require_relative 'GroupingAdapter.rb'
require 'mqtt/sub_handler'

module Xasin
module Telegram
	module MQTT
		class Server < GroupingAdapter
			attr_accessor :usernameList

			def initialize(httpCore, mqtt)
				super(httpCore);

				@mqtt = mqtt;
				setup_mqtt();
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

			def on_message(data, uID)
				@mqtt.publish_to "Telegram/#{uID}/Received", data.to_json;
			end
			def on_command(data, uID)
				@mqtt.publish_to "Telegram/#{uID}/Command", data.to_json;
			end
			def on_reply(data, uID)
				@mqtt.publish_to "Telegram/#{uID}/Reply", data.to_json;
			end
			def on_callback_pressed(data, uID)
				@mqtt.publish_to "Telegram/#{uID}/KeyboardPress", data.to_json
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
					elsif(data[:reply_gid])

					else

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
					rescue
					end
				end
			end
		end
	end
end
end
