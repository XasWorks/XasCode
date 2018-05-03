
require "mqtt/sub_handler"

module MQTT
	module Testing
		class SubHandler < MQTT::SubHandler
			attr_reader :caught_errors

			def call_interested(topic, data)
				@callbackList.each do |h|
					tMatch = SubHandler.getTopicMatch(topic, h.topic_split);
					if tMatch then h.offer(tMatch, data) end
				end
			end

			def raw_subscribe_to(_topic, qos: nil)
				if(@retained_topics[topic]) then
					publish_to(topic, @retained_topics[topic])
				end
			end

			def publish_to(topic, data, qos: nil, retain: false)
				@publish_queue << [topic, data];
				@retained_topics[topic] = data if retain;
			end

			def initialize()
				@caught_errors = Array.new();
				@callbackList = Array.new();

				@retained_topics = Hash.new();
			end
		end
	end
end
