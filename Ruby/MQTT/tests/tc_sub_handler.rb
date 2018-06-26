
require_relative 'tc_sub_testing.rb'

class Test_SubHandler < Minitest::Test
	def setup()
		@mqtt = MQTT::Testing::SubHandler.new(test_handler: self);
	end
	def teardown()
		@mqtt.full_reset();
	end

	def test_wildcard
		@mqtt.subscribe_to "Test/+/Card" do |data, tList|
			@tList = tList;
		end
		@mqtt.publish_to "Test/Wild/Card", "Data!"
		@mqtt.process_all

		assert_equal ["Wild"], @tList
		@tList = nil

		@mqtt.publish_to "Test/Wild/Card/Game", "Data!"
		@mqtt.process_all

		assert_nil @tList

		@mqtt.full_reset();
		@mqtt.subscribe_to "Test/#" do |data, tList|
			@tList = tList;
		end

		@mqtt.publish_to "Test/Wild/Card", "More data!"
		@mqtt.process_message

		assert_equal ["Wild", "Card"], @tList
	end

	def test_track
		@mqtt.track "TestChannel" do |newData, oldData|
			@newData = newData;
			@oldData = oldData;
		end

		assert_nil @newData
		assert_nil @oldData

		@mqtt.publish_to "TestChannel", "TestData"
		@mqtt.process_message
		assert_nil @oldData
		assert_equal "TestData", @newData

		@newData = nil;
		@mqtt.publish_to "TestChannel", "TestData"
		@mqtt.process_message
		assert_nil @newData

		@mqtt.publish_to "TestChannel", "NewData"
		@mqtt.process_message
		assert_equal @newData, "NewData"
		assert_equal @oldData, "TestData"

		assert_raises {
			@mqtt.track "TestChannel/+" do end
		}

		@mqtt.full_reset();
		@mqtt.publish_to "TestChannel", "NewData", retain: true;
		@mqtt.process_message

		@mqtt.track "TestChannel" do |newData, oldData|
			@newData = newData;
			@oldData = oldData;
		end

		assert_equal "NewData", @newData;
		assert_nil @oldData;
	end

	def test_wait_for
		refute (
			@mqtt.wait_for "NoDataHere", timeout: 0.1 do |data| end
		)

		waitThread = Thread.new do
			@mqtt.wait_for "SomeDataHere", timeout: 0.5 do |data|
				@data = data;
				true
			end
		end
		sleep 0.1

		@mqtt.publish_to "SomeDataHere", "FreshData";
		@mqtt.process_all

		waitThread.join
		assert_equal "FreshData", @data
	end
end
