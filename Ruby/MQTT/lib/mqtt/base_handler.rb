
require 'timeout'
require 'mqtt'
require 'colorize'

require 'xasin_logger'

module MQTT
	class BaseHandler
		include XasLogger::Mix

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
				tMatch = BaseHandler.getTopicMatch(topic, h.topic_split);
				if tMatch
					begin
						Timeout.timeout(10) {
							h.offer(tMatch, data)
						}
					rescue Timeout::Error
						x_loge("Timeout on callback #{h}");
					rescue => e
						x_logf("Uncaught error on #{h}");
						x_logf(e.inspect);
					end
					topicHasReceivers = true;
				end
			end

			@mqtt.unsubscribe(topic) unless topicHasReceivers;
		end
		private :call_interested

		def queue_packet(data)
			return if @destroying

			@packetQueueMutex.synchronize {
				@packetQueue << data;
				if(@packetQueue.size == 999)
					x_logf("Packet queue congested, dropping packets!");
				end
				if(@packetQueue.size > 1000)
					@packetQueue.shift
				end

				@publisherThread.run() if @publisherThreadWaiting;
			}
		end
		private :queue_packet

		# @!group Custom subscription handling

		# Unregister a subscription. Removes it from the callback list and
		# unsubscribes from the topic if no other subscriptions for it are present.
		# @param subObject [MQTT::Subscriptions::Subscription]
		#  The subscription-object to remove
		# @return void
		def unregister_subscription(subObject)
			raise ArgumentError, "Object is not a subscription!" unless subObject.is_a? MQTT::Subscriptions::Subscription
			return unless @callbackList.include? subObject;

			queue_packet({type: :unsub, topic: subObject.topic});
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
			queue_packet({type: :sub, topic: subObject.topic, qos: subObject.qos});
		end

		# @!endgroup

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
				x_logi("Logging out.")
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
					x_loge("Timed out, aborting!");
				else
					x_logi("Done");
				end
			end
		end
		private :ensure_clean_exit

		def attempt_packet_publish()
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
		end

		def mqtt_push_thread
			@push_error_count = 0;

			loop do
				@packetQueueMutex.synchronize {
					@publisherThreadWaiting = true;
				}
				x_logd("Push thread stopping")
				sleep 1
				x_logd("Push thread active")
				@packetQueueMutex.synchronize {
					@publisherThreadWaiting = false;
				}
				break if @destroying

				next unless @connected

				begin
					attempt_packet_publish();
				rescue MQTT::Exception, SocketError, SystemCallError, Timeout::Error => e
					x_loge("Push error!");
					x_loge(e.inspect);

					@push_error_count += 1;
					if(@push_error_count >= 10)
						@mqtt.disconnect();
					end

					sleep 0.5
				else
					@push_error_count = 0;
				end
			end

			x_logd("Push thread exited!");
		end
		private :mqtt_push_thread

		def mqtt_resub_thread
			loop do
				begin
					return if @destroying

					x_logw("Trying to reconnect...");
					Timeout.timeout(4) {
						@mqtt.connect()
					}
					x_logi("Connected!");
					@conChangeMutex.synchronize {
						@connected = true;
						@reconnectCount = 0;
					}

					@packetQueueMutex.synchronize {
						@publisherThread.run() if (@publisherThread && @publisherThreadWaiting)
					}

					x_logd("Sub thread reading...");
					@mqtt.get do |topic, message|
						call_interested(topic, message);
					end
				rescue MQTT::Exception, Timeout::Error, SocketError, SystemCallError
					x_loge("Disconnected!") if @connected
					@connected = false;
					@reconnectCount += 1;

					@conChangeMutex.unlock if @conChangeMutex.owned?
					@mqtt.clean_session = false;
					sleep [0.1, 0.5, 1, 1, 5, 5, 5, 10, 10, 10, 10][@reconnectCount] || 30;
				end
			end

			x_logd("Sub thread exited");
		end
		private :mqtt_resub_thread

		def destroy!()
			return if @destroying
			@destroying = true;

			unless @packetQueue.empty?
				x_logd "Finishing sending of MQTT messages ... "
				@publisherThread.run() if @publisherThreadWaiting
				begin
					Timeout.timeout(4) {
						until @packetQueue.empty? do
							sleep 0.05;
						end
					}
				rescue Timeout::Error
					x_logw "Not all messages were published";
				else
					x_logd "Publish clean finished"
				end
			end

			@publisherThread.run();
			@publisherThread.join();
			@listenerThread.kill();

			@mqtt.disconnect() if @connected

			ensure_clean_exit();

			x_logi("Fully disconnected!");
		end

		# Initialize a new MQTT::SubHandler
		# The handler immediately connects to the server, and begins receciving and sending.
		# @param mqttClient [String, MQTT::Client] Either a URI to connect to, or a MQTT::Client
		#  The URI can be of the form "mqtts://Password@User:URL:port".
		#  The MQTT client instance can be fully configured, as specified by the MQTT Gem. It must *not* already be connected!
		# @param jsonify [Boolean] Should Hashes and Arrays input into publish_to be converted to JSON?
		#  This can be useful to have one less .to_json call. Default is true.
		# @example Starting the handler
		#  mqtt = MQTT::SubHandler.new('mqtt.eclipse.org');
		#  mqtt = MQTT::SubHandler.new(MQTT::Client.new("Your.Client.Opts"))
		def initialize(mqttClient, logger: nil, **extra_opts)
			@callbackList = Array.new();
			if mqttClient.is_a? String
				@mqtt = MQTT::Client.new(mqttClient);
				@mqtt.clean_session = false unless extra_opts[:client_id].nil?
			else
				@mqtt = mqttClient;
			end

			init_x_log("MQTT #{@mqtt.host}", logger);
			self.log_level = Logger::INFO;

			@conChangeMutex = Mutex.new();
			@connected 		 = false;
			@reconnectCount = 0;

			@mqtt.client_id ||= extra_opts[:client_id] || MQTT::Client.generate_client_id("MQTT_Sub_", 8);

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

			begin
				Timeout.timeout(5) {
					until(@connected)
						sleep 0.1;
					end
				}
			rescue Timeout::Error
				x_loge("Broker did not connect!");
			end

			@publisherThread = Thread.new do
				mqtt_push_thread();
			end
			@publisherThread.abort_on_exception = true;

			at_exit {
				destroy!()
			}
		end
	end
end
