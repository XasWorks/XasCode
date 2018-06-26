
require_relative 'tc_sub_testing.rb'

class Test_SubHandler < Minitest::Test
	def setup()
		@mqtt = MQTT::Testing::SubHandler.new();
	end
	def teardown()
		@mqtt.full_reset();
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
	end
end
