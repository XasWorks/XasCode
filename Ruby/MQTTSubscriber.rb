
require 'mqtt'
require 'timeout'

require_relative 'Waitpoint.rb'

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

	def call_interested(topic, data)
		topicHasReceivers = false;
		@callbackList.each do |h|
			tMatch = MQTTSubs.getTopicMatch(topic, h.topic_split);
			if tMatch
				h.offer(tMatch, data)
				topicHasReceivers = true;
			end
		end

		@mqtt.unsubscribe(topic) unless topicHasReceivers;
	end

	def raw_subscribe_to(topic, qos: 1)
		begin
			@conChangeMutex.lock
			if not @connected then
				@subscribeQueue << [topic, qos];
				@conChangeMutex.unlock
			else
				@conChangeMutex.unlock
				@mqtt.subscribe(topic => qos);
			end
		rescue MQTT::Exception, SocketError, SystemCallError
			sleep 0.05;
			retry
		end
	end

	def unregister_subscription(subObject)
		return unless @callbackList.include? subObject;

		@callbackList.delete(subObject);
	end
	def register_subscription(subObject)
		return if @callbackList.include? subObject;

		@callbackList << subObject;
		raw_subscribe_to(subObject.topic, qos: subObject.qos);
	end
	def wait_for(topic, qos: 1, timeout: nil)
		subObject = MQTT::WaitpointSubscription.new(topic, qos);
		register_subscription(subObject);

		return_data = subObject.waitpoint.wait(timeout);

		unregister_subscription(subObject);

		return return_data;
	end
	def subscribe_to(topic, qos: 1, &callback)
		subObject = MQTT::Subscription.new(topic, qos, callback);
		register_subscription(subObject);

		return subObject;
	end
	alias subscribeTo subscribe_to


	def publish_to(topic, data, qos: 1, retain: false)
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
	alias publishTo publish_to

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
					@mqtt.subscribe(h[0] => h[1]);
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
					call_interested(topic, message);
				end
			rescue MQTT::Exception, Timeout::Error, SocketError, SystemCallError
				@connected = false;

				@conChangeMutex.unlock if @conChangeMutex.owned?
				@mqtt.clean_session=false;
				sleep 2
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
			until(@connected) do sleep 0.1; end
		}
		rescue Timeout::Error
		end
	end
end

module MQTT
	class Subscription
		attr_reader :topic
		attr_reader :qos
		attr_reader :topic_split

		def initialize(topic, qos, callback)
			@topic 		 = topic;
			@topic_split = MQTTSubs.getTopicSplit(topic);

			@qos 			= 0;
			@callback  	= callback;
		end
		def offer(topicList, data)
			@callback.call(topicList, data);
		end
	end

	class WaitpointSubscription
		attr_reader :topic
		attr_reader :qos
		attr_reader :topic_split

		attr_reader :waitpoint

		def initialize(topic, qos)
			@topic 		 = topic;
			@topic_split = MQTTSubs.getTopicSplit(topic);

			@qos 			 = 0;

			@waitpoint 	 = Xasin::Waitpoint.new();
		end

		def offer(topicList, data)
			@waitpoint.fire([topicList, data]);
		end
	end
end
