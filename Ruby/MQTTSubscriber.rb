
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

	def callInterested(topic, data)
		@callbackList.each do |h|
			tMatch = MQTTSubs.getTopicMatch(topic, h[:topic]);
			h[:cb].call(tMatch, data) if tMatch;
		end
	end

	def subscribeTo(topic, qos: 0, &callback)
		begin
			@conChangeMutex.lock
			if not @connected then
				@subscribeQueue << topic;
				@conChangeMutex.unlock
			else
				@conChangeMutex.unlock
				@mqtt.subscribe(topic);
			end
		rescue MQTT::Exception, SocketError, SystemCallError
			sleep 0.05;
			retry
		end

		@callbackList << {
			topic: 	MQTTSubs.getTopicSplit(topic),
			cb:		callback,
		}
	end

	def publishTo(topic, data, qos: 0, retain: false)
		raise ArgumentError, "Wrong symbol in topic: #{topic}" if topic =~ /[#\+]/

		begin
			@conChangeMutex.lock
			if not @connected then
				@publishQueue << {topic: topic, data: data, qos: qos, retain: retain} unless qos == 0
				@conChangeMutex.unlock
			else
				@conChangeMutex.unlock
				@mqtt.publish(topic, data, retain);
			end
		rescue MQTT::Exception, SocketError, SystemCallError
			sleep 0.05;
			retry
		end
	end

	def mqttResubThread
		while(true)
			begin
				Timeout::timeout(10) {
					@mqtt.connect()
				}
				@conChangeMutex.synchronize {
					@connected = true;
				}
				until @subscribeQueue.empty? do
					h = @subscribeQueue[-1];
					@mqtt.subscribe(h);
					@subscribedTopics[h] = true;
					@subscribeQueue.pop;
					sleep 0.01
				end
				until @publishQueue.empty? do
					h = @publishQueue[-1];
					@mqtt.publish(h[:topic], h[:data], h[:retain]);
					@publishQueue.pop;
					sleep 0.01
				end
				@mqtt.get do |topic, message|
					callInterested(topic, message);
				end
			rescue MQTT::Exception, Timeout::Error, SocketError, SystemCallError
				@connected = false;

				@conChangeMutex.unlock if @conChangeMutex.owned?
				@mqtt.clean_session=false;
				sleep 5
			end
		end
	end

	def lockAndListen()
		@listenerThread.join
	end

	def flush_pubqueue()
		until @publishQueue.empty? do
			sleep 0.05;
		end
	end

	def initialize(mqttClient, autoListen: true)
		@callbackList = Array.new();
		@mqtt = mqttClient;

		@conChangeMutex = Mutex.new();
		@connected 		= false;

		@mqtt.client_id = MQTT::Client.generate_client_id("MQTT_Sub_", length = 5) unless @mqtt.client_id

		@publishQueue 		= Array.new();
		@subscribeQueue 	= Array.new();
		@subscribedTopics 	= Hash.new();

		@listenerThread = Thread.new do
			if @mqtt.clean_session
				begin
					@mqtt.connect();
					@mqtt.disconnect();
				rescue MQTT::Exception
					sleep 1;
					retry
				rescue SocketError, SystemCallError
					sleep 5
					retry
				end
				@mqtt.clean_session=false;
			end

			mqttResubThread
		end
		@listenerThread.abort_on_exception = true;

		at_exit {
			flush_pubqueue
		}

		begin
		Timeout::timeout(10) {
			until(@connected) do sleep 0.5; end
		}
		rescue Timeout::Error
		end
	end
end
