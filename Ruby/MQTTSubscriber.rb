
require 'mqtt'

class MQTTSubs

	def self.getTopicSplit(topicName)
		return topicName.scan(/[^\/]+/);
	end

	def self.getTopicMatch(receivedTopicString, topicPattern)
		receivedTopicList = getTopicSplit receivedTopicString;

		outputTopicList = Array.new();

		return nil unless receivedTopicList.length >= topicPattern.length;

		topicPattern.each_index do |i|
			if(topicPattern[i] == "+") then
				outputTopicList << receivedTopicList[i];

			elsif(topicPattern[i] == "#") then
				outputTopicList.concat receivedTopicList[i..-1];
				return outputTopicList;

			elsif topicPattern[i] != receivedTopicList[i];
				return nil;

			end
		end

		return outputTopicList if topicPattern.length == receivedTopicList.length;
		return nil;
	end

	def subscribeTo(topic, qos = 0, &callback)
		begin
			@mqtt.subscribe(topic);
		rescue MQTT::Exception
		end
		@callbackList << {
			topic: 	MQTTSubs.getTopicSplit(topic),
			topicString: topic,
			cb:		callback,
		}
	end

	def callInterested(topic, data)
		@callbackList.each do |h|
			tMatch = MQTTSubs.getTopicMatch(topic, h[:topic]);
			h[:cb].call(tMatch, data) if tMatch;
		end
	end

	def publishTo(topic, data, qos: 0, retain: false)
		begin
			@mqtt.publish(topic, data, retain);
		rescue MQTT::Exception
			@publishQueue << {topic: topic, data: data, qos: qos, retain: retain} if qos > 0;
		end
	end

	def lockAndListen()
		while(true)
			begin
				@mqtt.connect() do |c|
					@callbackList.each do |h|
						c.subscribe(h[:topicString]);
					end
					until @publishQueue.empty? do
						h = @publishQueue.pop
						c.publish(h[:topic], h[:data], h[:retain]);
					end
					@mqtt.get do |topic, message|
						callInterested(topic, message);
					end
				end
			rescue MQTT::Exception
				@mqtt.clean_session=false;
				sleep 1
			end
		end
	end

	def initialize(mqttClient, autoListen: true)
		@callbackList = Array.new();
		@mqtt = mqttClient;

		@mqtt.client_id = MQTT::Client.generate_client_id("MQTT_Sub_", length = 5) unless @mqtt.client_id;
		begin
			@mqtt.clean_session=true;
			@mqtt.connect();
			@mqtt.disconnect();
			@mqtt.clean_session=false;
		rescue MQTT::Exception
		end

		@publishQueue = Array.new();

		if autoListen then
			@listenerThread = Thread.new do
				lockAndListen
			end
			@listenerThread.abort_on_exception = true;
		end
	end
end
