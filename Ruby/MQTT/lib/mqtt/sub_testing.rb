
require_relative "sub_handler.rb"
require 'securerandom'

module MQTT
	module Testing
		# This class is meant purely for testing.
		#  It completely removes the need for a external MQTT broker, and captures errors.
		#  Message processing can be done step-by-step, for better analysis of errors.
		#  Its interface is identical to the main class, making it indistinguishable.
		class SubHandler < MQTT::SubHandler
			attr_reader		:message_log
			attr_reader		:publish_queue
			attr_accessor 	:retained_topics

			attr_accessor  :test_handler

			def call_interested(topic, data)
				@callbackList.each do |h|
					tMatch = SubHandler.getTopicMatch(topic, h.topic_split);
					if tMatch then
						h.offer(tMatch, data)
					end
				end
			end
			private :call_interested

			def raw_subscribe_to(topic, qos: nil)
				@retained_topics.each do |retTopic, retData|
					if SubHandler.getTopicMatch(retTopic, SubHandler.get_topic_split(topic))
						publish_to(retTopic, retData)
					end
				end
			end
			private :raw_subscribe_to

			# Publish a message to topic.
			# @param topic [String] The topic to push to.
			# @param data [String] The data to be transmitted.
			# @note The published data is not immediately processed.
			#  Use process_message or process_all
			def publish_to(topic, data, qos: nil, retain: false)
				if(@jsonifyHashes and (data.is_a? Array or data.is_a? Hash))
					data = data.to_json
				else
					data = data.to_s;
				end

				@publish_queue 		<< [topic, data];
				@message_log[topic] 	<< data;
				@retained_topics[topic] = data if retain;
			end

			# Process a single MQTT message in queue, recording errors etc.
			def process_message
				return if @publish_queue.empty?
				packet = @publish_queue.pop
				call_interested(packet[0], packet[1]);
			end

			# Process all messages until the queue is empty.
			#  Do remember that the callbacks can publish new data, which then gets processed again!
			# @param max_loops [Integer] Amount of loops to do before aborting
			# @param error_on_loop [Boolean] Raise an error if too many loops happened?
			def process_all(max_loops: 500, error_on_loop: true)
				until @publish_queue.empty?
					process_message
					max_loops -= 1;

					if(max_loops == 0)
						if(error_on_loop)
							if(@test_handler)
								@test_handler.flunk("MQTT Loop recursion detected")
							else
								raise RuntimeError, "MQTT Loop recursion detected"
							end
						end
						return
					end
				end

				if(error_on_loop and @test_handler)
					@test_handler.pass("No MQTT Loop recursion.");
				end
			end

			def assert_garbage_resilient()
				@callbackList.each do |c|
					t = Array.new() {|a,k| a[k] = SecureRandom.random_bytes(100)}
					c.offer(t, SecureRandom.random_bytes(100));
				end

				if(@test_handler)
					@test_handler.pass("No garbage message problem detected.");
				end
			end

			# Prepare the code for the next test by cleaning out all queues.
			#  The list of callbacks is not affected
			# @param retained [Boolean, Hash<String, String>] Either a bool whether or not to clear retained messages,
			#  or a Hash with Topic-Keys containing String-Data to use.
			def prepare(retained: true)
				@publish_queue.clear();
				if(retained.is_a? Hash)
					@retained_topics = retained.clone
					@retained_topics.each do |topic, data|
						publish_to(topic, data);
					end
				elsif(retained)
					@retained_topics = Hash.new();
				end
				@message_log.clear()
			end

			# Perform a complete reset of the testing unit, clearing out all
			#  queues and removing all callbacks. This should mainly be used
			#  inside the teardown or setup routines, before creating a new
			#  testing instance, to prevent uncleaned garbage.
			def full_reset()
				@callbackList.clear();
				prepare();
			end

			# Initialize the test class
			# @param jsonify [Boolean] Whether or not Hashes and Arrays should be
			#  converted to JSON before sending.
			# @param test_handler [nil, MiniTest::Test] The test handler to use to report
			#  errors or pass sanity checks. Must support flunk and pass!
			def initialize(jsonify: true, test_handler: nil)
				@callbackList    = Array.new();
				@retained_topics = Hash.new();
				@publish_queue   = Queue.new();
				@message_log	  = Hash.new() do |h, key| h = Array.new() end;

				@trackerHash = Hash.new();

				@jsonifyHashes = jsonify;

				@test_handler = test_handler;
			end
		end
	end
end
