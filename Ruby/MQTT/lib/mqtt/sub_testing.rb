
require_relative "sub_handler.rb"

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

			def call_interested(topic, data)
				@callbackList.each do |h|
					tMatch = SubHandler.getTopicMatch(topic, h.topic_split);
					if tMatch then
						begin
							h.offer(tMatch, data)
						rescue StandardError => e
							if(@error_handler)
								@error_handler.call(e);
							else
								raise
							end
							return
						end
					end
				end
			end
			private :call_interested

			def raw_subscribe_to(topic, qos: nil)
				if(@retained_topics[topic]) then
					publish_to(topic, @retained_topics[topic])
				end
			end
			private :raw_subscribe_to

			# Publish a message to topic.
			# @param topic [String] The topic to push to.
			# @param data [String] The data to be transmitted.
			# @note The published data is not immediately processed.
			#  Use process_message or process_all
			def publish_to(topic, data, qos: nil, retain: false)
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
							raise RuntimeError, "MQTT Loop recursion detected!"
						end
						return
					end
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

			def initialize()
				@callbackList    = Array.new();
				@retained_topics = Hash.new();
				@publish_queue   = Queue.new();
				@message_log	  = Hash.new() do |h, key| h = Array.new() end;
			end

			def on_error(&handler)
				@error_handler = handler;
			end
		end
	end
end
