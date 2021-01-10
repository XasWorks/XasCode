

require 'json'

require_relative 'subscription_classes.rb'
require_relative 'base_handler.rb'

# @author Xasin
module MQTT
# A shortcut-function to quickly connect to the public Eclipse MQTT Broker.
# @return [MQTT::SubHandler] Sub-Handler connected to `mqtt.eclipse.org`
def self.Eclipse()
	@EclipseMQTT ||= SubHandler.new('mqtt.eclipse.org');
	return @EclipseMQTT;
end


class SubHandler < BaseHandler
	# Whether or not hashes and arrays should be converted to JSON when sending
	attr_accessor :jsonifyHashes

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
			x_logw("push with QOS > 1 was attempted, this is not supported yet!") unless $MQTTPubQOSWarned
			$MQTTPubQOSWarned = true;
		end

		queue_packet({type: :pub, topic: topic, data: data, qos: qos, retain: retain});
	end
	alias publishTo publish_to

	# Pause the main thread and wait for messages.
	# This is mainly useful when the code has set everything up, but doesn't just want to end.
	# "INT" is trapped, ensuring a smooth exit on Ctrl-C
	def lockAndListen()
		Signal.trap("INT") {
			exit 0
		}

		x_logi("Main thread paused.")
		Thread.stop();
	end
	alias lock_and_listen lockAndListen

	def initialize(mqttClient, jsonify: true, **extra_opts)
		super(mqttClient, **extra_opts);

		@jsonifyHashes = jsonify;
	end
end
end
