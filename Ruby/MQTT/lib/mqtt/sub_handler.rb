
require 'timeout'
require 'mqtt'

require 'mqtt/Waitpoint.rb'

module MQTT
class SubHandler
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
			tMatch = SubHandler.getTopicMatch(topic, h.topic_split);
			if tMatch
				h.offer(tMatch, data)
				topicHasReceivers = true;
			end
		end

		@mqtt.unsubscribe(topic) unless topicHasReceivers;
	end
	private :call_interested

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
	private :raw_subscribe_to

	def unregister_subscription(subObject)
		raise ArgumentError, "Object is not a subscription!" unless subObject.is_a? MQTT::Subscription
		return unless @callbackList.include? subObject;

		@callbackList.delete(subObject);
	end
	def register_subscription(subObject)
		raise ArgumentError, "Object is not a subscription!" unless subObject.is_a? MQTT::Subscription
		return if @callbackList.include? subObject;

		@callbackList << subObject;
		raw_subscribe_to(subObject.topic, qos: subObject.qos);
	end

	def wait_for(topic, qos: 1, timeout: nil)
		subObject = MQTT::WaitpointSubscription.new(topic, qos);
		register_subscription(subObject);

		if block_given? then
			begin
			Timeout::timeout(timeout) do
				loop do
					return_data = subObject.waitpoint.wait()[1];
					if yield(return_data[0], return_data[1]) then
						unregister_subscription(subObject);
						return true;
					end
				end
			end
			rescue Timeout::Error
				return false;
			end
		else
			return_data = subObject.waitpoint.wait(timeout);
		end

		unregister_subscription(subObject);
		return return_data;
	end
	def track(topic, qos: 1, &callback)
		unless(@trackerHash.has_key? topic)
			subObject = MQTT::ValueTrackerSubscription.new(topic, qos);
			register_subscription(subObject);

			@trackerHash[topic] = subObject;
		end

		@trackerHash[topic].attach(callback) if(callback)

		return @trackerHash[topic];
	end
	alias on_change track
	def subscribe_to(topic, qos: 1, &callback)
		subObject = MQTT::CallbackSubscription.new(topic, qos, callback);
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

	def mqtt_resub_thread
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
	private :mqtt_resub_thread

	def lockAndListen()
		Signal.trap("INT") {
			exit 0
		}

		puts "Main thread paused."
		Thread.stop();
	end
	def flush_pubqueue()
		puts "\n";
		if @publishQueue.empty? then
			puts "MQTT buffer empty, continuing."
		else
			print "Finishing sending of MQTT messages ... "
			begin
				Timeout::timeout(10) {
					until @publishQueue.empty? do
						sleep 0.05;
					end
				}
			rescue Timeout::Error
				puts "Timed out, aborting."
			else
				puts "Done."
			end
		end
	end

	def initialize(mqttClient, autoListen: true)
		@callbackList = Array.new();
		if mqttClient.is_a? String then
			@mqtt = MQTT::Client.new(mqttClient);
		else
			@mqtt = mqttClient;
		end

		@conChangeMutex = Mutex.new();
		@connected 		= false;

		@mqtt.client_id ||= MQTT::Client.generate_client_id("MQTT_Sub_", length = 8)

		@publishQueue 		= Array.new();
		@subscribeQueue 	= Array.new();
		@subscribedTopics 	= Hash.new();

		@trackerHash = Hash.new();

		@listenerThread = Thread.new do
			if @mqtt.clean_session
				@mqttWasStartedClean = true;
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

			mqtt_resub_thread
		end
		@listenerThread.abort_on_exception = true;

		at_exit {
			flush_pubqueue
			@listenerThread.kill();

			if(@mqttWasStartedClean) then
				print "Logging out of mqtt server... "
				begin
				Timeout::timeout(10) {
					begin
						@mqtt.clean_session = true;
						@mqtt.disconnect();
						@mqtt.connect();
					rescue MQTT::Exception, SocketError, SystemCallError
						sleep 1
						retry;
					end
				}
				rescue  Timeout::Error
					puts "Timed out, aborting!";
				else
					puts "Done."
				end
			end
		}

		begin
		Timeout::timeout(10) {
			until(@connected) do sleep 0.1; end
		}
		rescue Timeout::Error
		end
	end
end

class Subscription
	attr_reader :topic
	attr_reader :qos
	attr_reader :topic_split

	def initialize(topic, qos)
		@topic 		 = topic;
		@topic_split = SubHandler.getTopicSplit(topic);

		@qos 			= 0;
	end

	def offer() end
end

class CallbackSubscription < Subscription
	def initialize(topic, qos, callback)
		super(topic, qos);

		@callback  	= callback;
	end
	def offer(topicList, data)
		@callback.call(topicList, data);
	end
end

class WaitpointSubscription < Subscription
	attr_reader :waitpoint

	def initialize(topic, qos)
		super(topic, qos);

		@waitpoint 	 = Xasin::Waitpoint.new();
	end

	def offer(topicList, data)
		@waitpoint.fire([topicList, data]);
	end
end

class ValueTrackerSubscription < Subscription
	attr_reader :value

	def initialize(topic, qos = 1)
		raise ArgumentError, "Won't check values for wildcard topics! Topic: #{topic}" if topic =~ /[#\+]/
		super(topic, qos);

		@value = nil;

		@callbackList = Array.new();
	end

	def offer(topicList, data)
		return if data == @value;

		@callbackList.each do |cb|
			cb.call(data, @value);
		end
		@value = data;
	end

	def attach(callback)
		@callbackList << callback;
		callback.call(@value, nil) if(@value);
		return callback;
	end
	def remove(callback)
		@callbackList.delete callback;
	end
end
end
