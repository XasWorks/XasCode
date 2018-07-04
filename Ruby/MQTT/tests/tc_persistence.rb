
require 'minitest/autorun'

require_relative '../lib/mqtt/sub_testing.rb'
require_relative '../lib/mqtt/persistence.rb'

class TestPersistence < Minitest::Test
	def setup()
		@mqtt = MQTT::Testing::SubHandler.new(test_handler: self);
		@persistence = MQTT::Persistence.new(@mqtt);
	end
	def teardown()
		@mqtt.full_reset();
	end

	def test_pub_receive()
		@persistence.setup(:test_a);

		testHash = {"Test" => "Hash"};

		@persistence[:test_a] = testHash;
		@mqtt.process_all

		assert_equal testHash.to_json, @mqtt.retained_topics["Persistence/test_a"];

		testHash = {
			test: "Data",
			nr:   2,
		}

		@mqtt.publish_to "Persistence/test_a", testHash.to_json;
		@mqtt.process_all

		assert_equal testHash, @persistence[:test_a]
	end

	def test_retained_receive
		testHash = {test: "Data"}
		@mqtt.publish_to "Persistence/test_a", testHash.to_json, retain: true;
		@mqtt.process_all

		@persistence.setup(:test_a);

		assert_equal testHash, @persistence[:test_a]
	end

	def test_callbacks
		testHash = {test: "Data"}
		@mqtt.publish_to "Persistence/test_a", testHash.to_json, retain: true;

		assert_raises do
			@persistence.on_set(:test_a) do end
		end

		@persistence.setup(:test_a)
		@persistence.on_set(:test_a) do |newData, oldData|
			@setNewData = newData,
			@setOldData = oldData;
		end
		@persistence.on_change(:test_a) do |newData, oldData|
			@changeNewData = newData;
			@changeOldData = oldData;
		end

		assert_equal testHash, @setNewData;
		[@setOldData, @changeNewData, @changeOldData].each do |d| assert_nil d; end
	end
end
