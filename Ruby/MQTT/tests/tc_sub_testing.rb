
require_relative '../lib/mqtt/sub_testing.rb'
require 'minitest/autorun'

XasLogger.loggers = [];

=begin

class Test_SubHandlerTest < Minitest::Test
	def setup()
		@mqtt = MQTT::SubHandler.new('localhost');
	end
	def teardown()
		@mqtt.destroy!();
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

	def test_retained
		@mqtt.publish_to "Unrelated/Retain", "Test", retain: true;
		@mqtt.publish_to "Retained/One", "Test1!", retain: true;
		@mqtt.publish_to "Retained/Two", "Test2!", retain: true;

		@mqtt.process_all();

		caughtData = Hash.new();
		@mqtt.subscribe_to "Retained/+" do |data, tSplit|
			caughtData[tSplit[0]] = data;
		end
		assert_equal 2, @mqtt.publish_queue.length

		@mqtt.process_all();

		assert_equal({"One" => "Test1!", "Two" => "Test2!"}, caughtData)
	end
end

=end
