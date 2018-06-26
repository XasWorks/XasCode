
require_relative '../lib/mqtt/sub_testing.rb'
require 'minitest/autorun'

class Test_SubHandlerTest < Minitest::Test
	def setup()
		@mqtt = MQTT::Testing::SubHandler.new();
	end

	def teardown()
		@mqtt.full_reset();
	end

	def test_subscribe_unsubscribe()
		subObj = @mqtt.subscribe_to "TestChannel" do |data|
			@received = data;
		end
		@mqtt.publish_to "TestChannel", "TestData";

		assert_nil @received
		@mqtt.process_message
		assert_equal "TestData", @received

		@received = nil;
		@mqtt.unregister_subscription(subObj);
		@mqtt.publish_to "TestChannel", "TestData";
		@mqtt.process_message
		assert_nil @received
	end

	def test_error_raise
		@mqtt.subscribe_to "TestChannel" do |data|
			raise "TestRaise!"
		end
		@mqtt.publish_to "TestChannel", "ErrorData"

		assert_raises {
			@mqtt.process_message();
		}
	end

	def test_loop_detect()
		@mqtt.subscribe_to "TestChannel" do |data|
			@mqtt.publish_to "TestChannel", "More data!"
		end
		@mqtt.publish_to "TestChannel", "Some data"

		assert_raises {
			@mqtt.process_all()
		}
	end

	def test_garbage_testing
		@mqtt.subscribe_to "GarbageResilientChannel" do |data| end
		@mqtt.assert_garbage_resilient();

		@mqtt.subscribe_to "NotGarbageResilient" do |data|
			raise "Nope! D:"
		end

		assert_raises {
			@mqtt.assert_garbage_resilient();
		}
	end
end
