
require 'timeout'
require 'mqtt'
require 'json'

require 'colorize'

require_relative 'subscription_classes.rb'

# @author Xasin
module MQTT
# A shortcut-function to quickly connect to the public Eclipse MQTT Broker.
# @return [MQTT::SubHandler] Sub-Handler connected to `iot.eclipse.org`
def self.Eclipse()
	@EclipseMQTT ||= SubHandler.new('iot.eclipse.org');
	return @EclipseMQTT;
end


class SubHandler
	# Whether or not hashes and arrays should be converted to JSON when sending
	attr_accessor :jsonifyHashes

	# Split a Topic into a Topic-Array
	# @param topicName [String] The string topic which to split
	# @return [Array<String>] A list of individual topic-branches
	# @note This function is mainly used for background processing.
	def self.get_topic_split(topicName)
		return topicName.scan(/[^\/]+/);
	end

	# Match a topic string to a topic pattern
	# @param receivedTopicString [String] The string (as
	#  returned by MQTT.get) to compare
	# @param topicPattern [Array<String>] The Topic-Array (as
	#  returned by .get_topic_split) to compare against
	# @return [nil, Array<String>] Nil if no match was found.
	#  An Array of matched wildcard topic branches (can be empty) when
	#  successfully matched
	# @note (see .get_topic_split)
	def self.getTopicMatch(receivedTopicString, topicPattern)
		receivedTopicList = get_topic_split receivedTopicString;

		outputTopicList = Array.new();

		return nil unless receivedTopicList.length >= topicPattern.length;

		topicPattern.each_index do |i|
			if(topicPattern[i] == "+")
				outputTopicList << receivedTopicList[i];

			elsif(topicPattern[i] == "#")
				outputTopicList.concat receivedTopicList[i..-1];
				return outputTopicList;

			elsif topicPattern[i] != receivedTopicList[i];
				return nil;

			end
		end

		return outputTopicList if topicPattern.length == receivedTopicList.length;
		return nil;
	end

	# Call all existing callbacks whose topic-list matches `topic`
	def call_interested(topic, data)
		topicHasReceivers = false;
		@callbackList.each do |h|
			tMatch = SubHandler.getTopicMatch(topic, h.topic_split);
			if tMatch
				begin
					Timeout.timeout(5) {
						h.offer(tMatch, data)
					}
				rescue Timeout::Error
					STDERR.puts "MQTT: Callback Timeout #{h}".red
				end
				topicHasReceivers = true;
			end
		end

		@mqtt.unsubscribe(topic) unless topicHasReceivers;
	end
	private :call_interested

	def queue_packet(data)
		@packetQueueMutex.synchronize {
			@packetQueue << data;
			@packetQueue.shift if @packetQueue.size > 100

			@publisherThread.run() if @publisherThreadWaiting;
		}
	end

	# Handle sending a subscription-message to the server
	def raw_subscribe_to(topic, qos: 1)
		queue_packet({topic: topic, qos: qos, type: :sub});
	end
	private :raw_subscribe_to

	# @!group Custom subscription handling

	# Unregister a subscription. Removes it from the callback list and
	# unsubscribes from the topic if no other subscriptions for it are present.
	# @param subObject [MQTT::Subscriptions::Subscription]
	#  The subscription-object to remove
	# @return void
	def unregister_subscription(subObject)
		raise ArgumentError, "Object is not a subscription!" unless subObject.is_a? MQTT::Subscriptions::Subscription
		return unless @callbackList.include? subObject;

		@callbackList.delete(subObject);
	end
	# Register a custom subscription, and send a subscription message to the server.
	# @param subObject [MQTT::Subscriptions::Subscription]
	#  An instance of a MQTT Subscription object
	# @return void
	def register_subscription(subObject)
		raise ArgumentError, "Object is not a subscription!" unless subObject.is_a? MQTT::Subscriptions::Subscription
		return if @callbackList.include? subObject;

		@callbackList << subObject;
		raw_subscribe_to(subObject.topic, qos: subObject.qos);
	end

	# @!group Subscribing

	# Synchronously wait for data.
	# It waits for a message on `topic`, optionally letting a block
	# check the data for validity, and optionally aborting after a timeout
	# @param topic [String] The MQTT-Topic to wait for
	# @param timeout [nil, Integer] The optional timeout after which to abort
	# @param qos [nil, Integer] The QoS for this subscription
	# @return [Boolean] True if the block returned true, False if the code timed-out
	# @yieldparam data [String] The data received via MQTT
	# @yieldparam topicList [Array<String>] The wildcard topic branches matched.
	# @yieldreturn [Boolean] Whether or not the data was sufficient, and capture should be stopped.
	def wait_for(topic, qos: 1, timeout: nil)
		unless block_given?
			raise ArgumentError, "A block for data-processing needs to be passed!"
		end

		subObject = MQTT::Subscriptions::WaitpointSubscription.new(topic, qos);
		register_subscription(subObject);

		begin
		Timeout.timeout(timeout) do
			loop do
				return_data = subObject.waitpoint.wait()[1];
				if yield(return_data[0], return_data[1])
					return true;
				end
			end
		end
		rescue Timeout::Error
			return false;
		ensure
			unregister_subscription(subObject);
		end
	end

	# Track data changes for a topic in the background.
	# With no callback given, the returned object can be used to get the last
	# received raw data string.
	# With a callback given, the callback will be called whenever a change in data
	# is detected.
	# @param topic [String] The MQTT-Topic to track for data. Can be a Wildcard.
	# @param qos [nil, Integer] The QoS to use for the subscription
	# @yieldparam data [String] The new (changed) data received from MQTT.
	# @yieldreturn [void]
	# @return [MQTT::Subscriptions::ValueTrackerSubscription]
	#  The tracker-object. Can be used to unsubscribe.
	#  More importantly, `tracker.value` can be used to fetch the last received data.
	def track(topic, qos: 1, &callback)
		unless(@trackerHash.has_key? topic)
			subObject = MQTT::Subscriptions::ValueTrackerSubscription.new(topic, qos);
			register_subscription(subObject);

			@trackerHash[topic] = subObject;
		end

		@trackerHash[topic].attach(callback) if(callback)

		return @trackerHash[topic];
	end
	alias on_change track

	# Attach a callback to a MQTT Topic or wildcard.
	# The callback will be saved, and asynchronously executed whenever a message
	# from a matching topic (including wildcards) is received.
	# @param topic [String] The MQTT-Topic to subscribe to. Can be a Wildcard.
	# @param qos [nil, Integer] The QoS for the subscription. Currently not used!
	# @yieldparam data [String] The raw MQTT data received from the MQTT server
	# @yieldparam topicList [Array<String>] An array of topic-branches corresponding to wildcard matches.
	#  Can be empty if no wildcard was used!
	# @yieldreturn [void]
	# @return [MQTT::Subscriptions::CallbackSubscription] The Subscription-Object corresponding to this callback.
	#  Mainly used by the .unregister_subscription function to unsubscribe.
	def subscribe_to(topic, qos: 1, &callback)
		subObject = MQTT::Subscriptions::CallbackSubscription.new(topic, qos, callback);
		register_subscription(subObject);

		return subObject;
	end
	alias subscribeTo subscribe_to

	# @!endgroup

	# Publish a message to topic.
	# @param topic [String] The topic to push to.
	# @param data [String] The data to be transmitted.
	# @param qos [nil, Numeric] QoS for the publish. Currently not fully supported by the mqtt gem.
	# @param retain [nil, Boolean] retain-flag for the publish.
	def publish_to(topic, data, qos: 1, retain: false)
		raise ArgumentError, "Wrong symbol in topic: #{topic}" if topic =~ /[#\+]/

		if(@jsonifyHashes and (data.is_a? Array or data.is_a? Hash))
			data = data.to_json
		end

		if(qos > 1)
			qos = 1
			STDERR.puts("MQTT push with QOS > 1 was attempted, this is not supported yet!".yellow) unless $MQTTPubQOSWarned

			$MQTTPubQOSWarned = true;
		end

		queue_packet({type: :pub, topic: topic, data: data, qos: qos, retain: retain});
	end
	alias publishTo publish_to

	def ensure_clean_start()
		@mqttWasStartedClean = @mqtt.clean_session
		if @mqttWasStartedClean
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
	end
	private :ensure_clean_start

	def ensure_clean_exit()
		if(@mqttWasStartedClean)
			print "Logging out of mqtt server... "
			begin
			Timeout.timeout(3) {
				begin
					@mqtt.clean_session = true;
					@mqtt.disconnect();
					@mqtt.connect();
				rescue MQTT::Exception, SocketError, SystemCallError
					sleep 0.3
					retry;
				end
			}
			rescue  Timeout::Error
				puts "Timed out, aborting!";
			else
				puts "Done."
			end
		end
	end
	private :ensure_clean_exit

	def mqtt_push_thread
		loop do
			@packetQueueMutex.synchronize {
				@publisherThreadWaiting = true;
			}
			Thread.stop();
			@packetQueueMutex.synchronize {
				@publisherThreadWaiting = false;
			}

			next unless @connected

			begin
				until @packetQueue.empty? do
					h = nil;
					@packetQueueMutex.synchronize {
						h = @packetQueue[0];
					}
					Timeout.timeout(3) {
						if(h[:type] == :sub)
							@mqtt.subscribe(h[:topic] => h[:qos]);
						elsif(h[:type] == :pub)
							@mqtt.publish(h[:topic], h[:data], h[:retain], h[:qos]);
						end
					}
					@packetQueueMutex.synchronize {
						@packetQueue.shift();
					}
				end
			rescue MQTT::Exception, SocketError, SystemCallError, Timeout::Error => e
					STDERR.puts("MQTT: #{@mqtt.host} push error, disconnecting!".red) if @connected
					STDERR.puts(e.inspect);

					sleep 1
			end
		end
	end
	private :mqtt_push_thread

	def mqtt_resub_thread
		loop do
			begin
				STDERR.puts("MQTT: #{@mqtt.host} trying reconnect...".yellow)
				Timeout.timeout(4) {
					@mqtt.connect()
				}
				STDERR.puts("MQTT: #{@mqtt.host} connected!".green)
				@conChangeMutex.synchronize {
					@connected = true;
				}

				@packetQueueMutex.synchronize {
					@publisherThread.run() if (@publisherThread && @publisherThreadWaiting)
				}

				@mqtt.get do |topic, message|
					call_interested(topic, message);
				end
			rescue MQTT::Exception, Timeout::Error, SocketError, SystemCallError
				STDERR.puts("MQTT: #{@mqtt.host} disconnected!".red) if @connected
				@connected = false;

				@conChangeMutex.unlock if @conChangeMutex.owned?
				@mqtt.clean_session = false;
				sleep 2
			end
		end
	end
	private :mqtt_resub_thread

	# Pause the main thread and wait for messages.
	# This is mainly useful when the code has set everything up, but doesn't just want to end.
	# "INT" is trapped, ensuring a smooth exit on Ctrl-C
	def lockAndListen()
		Signal.trap("INT") {
			exit 0
		}

		puts "Main thread paused."
		Thread.stop();
	end
	def flush_pubqueue()
		unless @packetQueue.empty?
			print "Finishing sending of MQTT messages ... "
			begin
				Timeout.timeout(4) {
					until @packetQueue.empty? do
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
	private :flush_pubqueue

	# Initialize a new MQTT::SubHandler
	# The handler immediately connects to the server, and begins receciving and sending.
	# @param mqttClient [String, MQTT::Client] Either a URI to connect to, or a MQTT::Client
	#  The URI can be of the form "mqtts://Password@User:URL:port".
	#  The MQTT client instance can be fully configured, as specified by the MQTT Gem. It must *not* already be connected!
	# @param jsonify [Boolean] Should Hashes and Arrays input into publish_to be converted to JSON?
	#  This can be useful to have one less .to_json call. Default is true.
	# @example Starting the handler
	#  mqtt = MQTT::SubHandler.new('iot.eclipse.org');
	#  mqtt = MQTT::SubHandler.new(MQTT::Client.new("Your.Client.Opts"))
	def initialize(mqttClient, jsonify: true)
		@callbackList = Array.new();
		if mqttClient.is_a? String
			@mqtt = MQTT::Client.new(mqttClient);
		else
			@mqtt = mqttClient;
		end

		@jsonifyHashes = jsonify;

		@conChangeMutex = Mutex.new();
		@connected 		= false;

		@mqtt.client_id ||= MQTT::Client.generate_client_id("MQTT_Sub_", 8);

		@packetQueue = Array.new();
		@packetQueueMutex = Mutex.new();
		@publisherThreadWaiting = false;

		@subscribedTopics 	= Hash.new();

		@trackerHash = Hash.new();

		@listenerThread = Thread.new do
			ensure_clean_start();
			mqtt_resub_thread();
		end
		@listenerThread.abort_on_exception = true;


		@publisherThread = Thread.new do
			mqtt_push_thread();
		end
		@publisherThread.abort_on_exception = true;

		at_exit {
			flush_pubqueue();
			@connected = false;
			@listenerThread.kill();
			ensure_clean_exit();
		}
	end
end
end
