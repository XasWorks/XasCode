
require 'mqtt'
require 'timeout'

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
		unless @subscribedTopics[topic] then
			begin
				@mqtt.subscribe(topic);
				@subscribedTopics[topic] = true;
			rescue MQTT::Exception
				@subscribeQueue << topic;
			end
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
		rescue MQTT::Exception
			@publishQueue << {topic: topic, data: data, qos: qos, retain: retain} if qos > 0;
		end
	end

	def lockAndListen()
		while(true)
			begin
				Timeout::timeout(10) {
					@mqtt.connect()
				}
				until @subscribeQueue.empty? do
					h = @subscribeQueue[-1];
					@mqtt.subscribe(h);
					@subscribedTopics[h] = true;
					@subscribeQueue.pop;
					sleep 0.05
				end
				until @publishQueue.empty? do
					h = @publishQueue[-1];
					@mqtt.publish(h[:topic], h[:data], h[:retain]);
					@publishQueue.pop;
					sleep 0.05
				end
				@mqtt.get do |topic, message|
					callInterested(topic, message);
				end
			rescue MQTT::Exception, Timeout::Error
				@mqtt.clean_session=false;
				sleep 5
			end
		end
	end

	def initialize(mqttClient, autoListen: true)
		@callbackList = Array.new();
		@mqtt = mqttClient;

		@mqtt.client_id = MQTT::Client.generate_client_id("MQTT_Sub_", length = 5) unless @mqtt.client_id;

		@mqtt.clean_session=true;
		@mqtt.connect();
		@mqtt.disconnect();
		@mqtt.clean_session=false;

		@publishQueue = Array.new();
		@subscribeQueue 	= Array.new();
		@subscribedTopics = Hash.new();

		if autoListen then
			@listenerThread = Thread.new do
				lockAndListen
			end
			@listenerThread.abort_on_exception = true;
		end
	end
end
