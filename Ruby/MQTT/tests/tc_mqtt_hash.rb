
require 'minitest/autorun'

require_relative '../lib/mqtt/mqtt_hash.rb'
require_relative '../lib/mqtt/sub_testing.rb'

class TC_Hash < Minitest::Test
	def setup
		@mqtt = MQTT::SubHandler.new('localhost');
	end

	def teardown()
		@mqtt.destroy!();
	end

	def test_single_layer
		tHash = MQTT::TXHash.new(@mqtt, "Test/Topic");

		@mqtt.publish_to "Test/Topic", "", retain: true
		sleep 0.3

		pubCount = 0;
		publishedObject = nil;
		@mqtt.subscribe_to "Test/Topic" do |data, tSplit|
			pubCount += 1;
			publishedObject = data;
		end

		tHash["Test"] 	= "Test1";
		tHash["Test2"]	= {and: "A", test: "!"};

		sleep 0.3

		assert_equal pubCount, 2,
			"Publish-length unexpected!"

		assert_equal publishedObject, tHash.hash.to_json,
			"Published hash did not match raw hash JSON"


		tHash.hash = {test: "One", two: "Three"}

		sleep 0.3

		assert_equal publishedObject, tHash.hash.to_json,
			"Published hash did not match raw hash JSON"
	end

	def test_single_wildcard
		tHash = MQTT::TXHash.new(@mqtt, "TestCase2/Topic/+");

		@retainedHash = Hash.new();
		@mqtt.subscribe_to "TestCase2/Topic/+" do |data, topics|
			@retainedHash[topics[0]] = data;
		end

		tHash["Test"] 	= "First Test";
		tHash["Test2"] = "Second Test";

		sleep 0.3;

		tHash.hash.each do |key, val|
			assert_equal val.to_json, @retainedHash[key.to_s],
				"Key #{key} did not get published properly!"
		end

		@retainedHash = Hash.new();

		tHash.hash = {Test: "Case", multi: {hash: "Test", test: "fire"}}

		sleep 0.3

		tHash.hash.each do |key, val|
			assert_equal val.to_json, @retainedHash[key.to_s],
				"Key #{key} did not get published properly!"
		end
	end
end
