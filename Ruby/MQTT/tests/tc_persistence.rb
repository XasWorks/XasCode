
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
		@mqtt.process_all

		assert_raises do
			@persistence.on_set(:test_a) do end
		end

		@persistence.setup(:test_a)
		@persistence.on_set(:test_a) do |newData, oldData|
			@setNewData = newData;
			@setOldData = oldData;
		end
		@persistence.on_change(:test_a) do |newData, oldData|
			@changeNewData = newData;
			@changeOldData = oldData;
		end

		assert_equal testHash, @setNewData;
		[@setOldData, @changeNewData, @changeOldData].each do |d| assert_nil d; end

		newTestHash = {new: "HashData"}
		@persistence[:test_a] = newTestHash;

		assert_equal newTestHash, @persistence[:test_a];

		assert_equal newTestHash, @setNewData;
		assert_equal newTestHash, @changeNewData;

		assert_equal testHash, @setOldData;
		assert_equal testHash, @changeOldData;

		testHash = newTestHash;
		newTestHash = {andAnother: "newHash"};
		@mqtt.publish_to "Persistence/test_a", newTestHash.to_json;
		@mqtt.process_all

		assert_equal newTestHash, @persistence[:test_a];

		assert_equal newTestHash, @setNewData;
		assert_equal newTestHash, @changeNewData;

		assert_equal testHash, @setOldData;
		assert_equal testHash, @changeOldData;
	end

	def test_conversion_time
		testTime = Time.at(rand(0..99999999));

		@persistence.setup(:test_a, Time);
		@persistence[:test_a] = testTime;
		@mqtt.process_all();
		assert_equal testTime.to_i.to_json, @mqtt.retained_topics["Persistence/test_a"];

		@persistence[:test_a] = nil;
		@mqtt.process_all();
		assert_equal nil.to_json, @mqtt.retained_topics["Persistence/test_a"];


		@mqtt.publish_to "Persistence/test_a", testTime.to_i.to_json
		@mqtt.process_all();
		assert_equal testTime, @persistence[:test_a]

		@mqtt.publish_to "Persistence/test_a", nil.to_json;
		@mqtt.process_all();
		assert_nil @persistence[:test_a];
	end

	def test_custom_conversion
		customClass = Class.new do
			attr_accessor :firstValue, :secondValue

			def self.from_mqtt_string(data)
				nInstance = new();
				nInstance.update_from_mqtt(data);
			end

			def update_from_mqtt(data)
				data = JSON.parse(data);
				@firstValue = data["first"];
				@secondValue = data["second"];
			end

			def to_mqtt_string()
				{
					first: @firstValue,
					second: @secondValue,
				}.to_json
			end
		end

		cInstance = customClass.new();
		@persistence.setup(:test_custom, customClass);

		cInstance.firstValue = "One test!"
		cInstance.secondValue = "Second test!"
		@persistence[:test_custom] = cInstance;

		assert_equal cInstance.to_mqtt_string, @mqtt.retained_topics["Persistence/test_custom"];

		@mqtt.publish_to "Persistence/test_custom", {
			first: "Test 1"
		}
		@mqtt.process_all();

		assert_equal "Test 1", cInstance.firstValue
		assert_nil   cInstance.secondValue
	end
end
