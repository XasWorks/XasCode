
require_relative 'Waitpoint.rb'

module MQTT
	module Subscriptions
		# @abstract Basis for custom MQTT::Subcription objects
		class Subscription
			attr_reader :topic
			attr_reader :qos
			attr_reader :topic_split

			def initialize(topic, _qos)
				@topic 		 = topic;
				@topic_split = SubHandler.get_topic_split(topic);

				@qos 			= 0;
			end

			def offer(topicList, data) end
		end

		class CallbackSubscription < Subscription
			def initialize(topic, qos, callback)
				super(topic, qos);

				@callback  	= callback;
			end
			def offer(topicList, data)
				@callback.call(data, topicList);
			end
		end

		class WaitpointSubscription < Subscription
			attr_reader :waitpoint

			def initialize(topic, qos)
				super(topic, qos);

				@waitpoint 	 = Xasin::Waitpoint.new();
			end

			def offer(topicList, data)
				@waitpoint.fire([data, topicList]);
			end
		end

		class ValueTrackerSubscription < Subscription
			attr_reader :value

			def initialize(topic, qos = 1)
				raise ArgumentError, "Tracking of topic wildcards is prohibited! Topic: #{topic}" if topic =~ /[#\+]/
				super(topic, qos);

				@value = nil;

				@callbackList = Array.new();
			end

			def offer(_topicList, data)
				return if data == @value;
				oldValue = @value;
				@value = data;

				@callbackList.each do |cb|
					cb.call(data, oldValue);
				end
			end

			def attach(callback)
				@callbackList << callback;
				callback.call(@value, nil) if(@value);
				return callback;
			end
			def detach(callback)
				@callbackList.delete callback;
			end
		end
	end
end
