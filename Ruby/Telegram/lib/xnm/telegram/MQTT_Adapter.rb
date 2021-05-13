
require_relative 'GroupingAdapter.rb'
require 'mqtt/sub_handler'

module XNM
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
						data = { text: data }
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
				super(data, uID);
				@mqtt.publish_to "Telegram/#{uID}/Received", data.to_json;
			end
			def on_command(data, uID)
				super(data, uID);
				@mqtt.publish_to "Telegram/#{uID}/Command", data.to_json;
			end
			def on_reply(data, uID)
				super(data, uID);
				@mqtt.publish_to "Telegram/#{uID}/Reply", data.to_json;
			end
			def on_callback_pressed(data, uID)
				super(data, uID);
				@mqtt.publish_to "Telegram/#{uID}/KeyboardPress", data.to_json
			end
		end
	end
end
end
