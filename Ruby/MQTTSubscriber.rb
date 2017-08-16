
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
		rescue MQTT::Error
		end
	end

	def lockAndListen()
		while(true)
			begin
				topic, message = @mqtt.get
			rescue MQTT::Error
				sleep 1
			end

			callInterested(topic, message);
		end
	end

	def initialize(mqttClient, autoListen: true)
		@callbackList = Array.new();
		@mqtt = mqttClient;

		if autoListen then
			Thread.new do
				lockAndListen
			end
		end
	end
end
