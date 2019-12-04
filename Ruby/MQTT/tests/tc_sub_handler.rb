
require_relative 'tc_sub_testing.rb'

class Test_SubHandler < Minitest::Test
	def setup()
		@mqtt = MQTT::SubHandler.new('localhost');
	end
	def teardown()
		@mqtt.destroy!();
	end


	def test_wildcard
		@mqtt.subscribe_to "Test/+/Card" do |data, tList|
			@tList = tList;
		end
		@mqtt.publish_to "Test/Wild/Card", "Data!"

		sleep 2

		assert_equal ["Wild"], @tList
		@tList = nil

		@mqtt.publish_to "Test/Wild/Card/Game", "Data!"

		sleep 2

		assert_nil @tList

		@mqtt.subscribe_to "Test2/#" do |data, tList|
			@tList = tList;
		end

		@mqtt.publish_to "Test2/Wild/Card", "More data!"

		sleep 2

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
		sleep 2

		assert_nil @oldData
		assert_equal "TestData", @newData

		@newData = nil;
		@mqtt.publish_to "TestChannel", "TestData"
		sleep 2

		assert_nil @newData

		@mqtt.publish_to "TestChannel", "NewData"
		sleep 2

		assert_equal @newData, "NewData"
		assert_equal @oldData, "TestData"

		assert_raises {
			@mqtt.track "TestChannel/+" do end
		}

		@mqtt.publish_to "TestChannel2", "MoreNewData!", retain: true;

		@mqtt.track "TestChannel2" do |newData, oldData|
			@newData = newData;
			@oldData = oldData;
		end

		sleep 2

		assert_equal "MoreNewData!", @newData;
		assert_nil @oldData;
	end

	def test_wait_for
		refute (
			@mqtt.wait_for "NoDataHere", timeout: 0.5 do |data| end
		)

		waitThread = Thread.new do
			@mqtt.wait_for "SomeDataHere", timeout: 2 do |data|
				@data = data;
				true
			end
		end
		sleep 0.1

		@mqtt.publish_to "SomeDataHere", "FreshData";
		sleep 2

		waitThread.join
		assert_equal "FreshData", @data
	end

	def test_binary_data
		unpacketData = [1, 2, 3];
		packedData = unpacketData.pack("L3");

		receivedRaw = nil;
		receivedUnpacked = nil;
		@mqtt.subscribe_to "Test/BinaryData" do |data|
			receivedRaw = data;
			receivedUnpacked = receivedRaw.unpack("L3");
		end

		sleep 0.3

		@mqtt.publish_to "Test/BinaryData", packedData;

		sleep 0.8

		assert_equal unpacketData, receivedUnpacked;
		assert_equal packedData, receivedRaw;
	end
end
