
require 'minitest/autorun'

require_relative '../lib/mqtt/mqtt_hash.rb'
require_relative '../lib/mqtt/sub_testing.rb'

class TC_Hash < Minitest::Test
	def setup
		@mqtt ||= MQTT::Testing::SubHandler.new();
		@mqtt.prepare();
	end

	def test_single_layer
		tHash = MQTT::TXHash.new(@mqtt, "Test/Topic");

		tHash["Test"] 	= "Test1";
		tHash["Test2"]	= {and: "A", test: "!"};

		assert_equal @mqtt.publish_queue.length, 2,
			"Publish-length unexpected!"
		@mqtt.process_all
		assert_equal @mqtt.retained_topics["Test/Topic"], tHash.hash.to_json,
			"Published hash did not match raw hash JSON"


		tHash.hash = {test: "One", two: "Three"}
		@mqtt.process_all
		assert_equal @mqtt.retained_topics["Test/Topic"], tHash.hash.to_json,
			"Published hash did not match raw hash JSON"
	end

	def test_single_wildcard
		tHash = MQTT::TXHash.new(@mqtt, "TestCase2/Topic/+");

		tHash["Test"] 	= "First Test";
		tHash["Test2"] = "Second Test";

		@mqtt.process_all
		tHash.hash.each do |key, val|
			assert_equal @mqtt.retained_topics["TestCase2/Topic/#{key}"], val.to_json,
				"Key #{key} did not get published properly!"
		end

		tHash.hash = {Test: "Case", multi: {hash: "Test", test: "fire"}}

		@mqtt.process_all
		tHash.hash.each do |key, val|
			assert_equal @mqtt.retained_topics["TestCase2/Topic/#{key}"], val.to_json,
				"Key #{key} did not get published properly!"
		end
	end
end
