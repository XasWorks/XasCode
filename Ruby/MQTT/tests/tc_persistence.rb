
require 'minitest/autorun'

require_relative '../lib/mqtt/sub_testing.rb'
require_relative '../lib/mqtt/persistence.rb'

class TestPersistence < Minitest::Test
	def setup
		@mqtt = MQTT::SubHandler.new('localhost');
		@persistence = MQTT::Persistence.new(@mqtt);
	end

	def teardown()
		@mqtt.destroy!();
	end


	def test_pub_receive()
		retainedValue = nil;
		@mqtt.subscribe_to "Persistence/test_a" do |data| retainedValue = data; end

		@persistence.setup(:test_a);

		testHash = {"Test" => "Hash"};

		@persistence[:test_a] = testHash;

		sleep 0.3

		assert_equal testHash.to_json, retainedValue;

		testHash = {
			test: "Data",
			nr:   2,
		}

		@mqtt.publish_to "Persistence/test_a", testHash.to_json;

		sleep 0.3

		assert_equal testHash, @persistence[:test_a]
	end

	def test_retained_receive
		testHash = {test: "Data"}
		@mqtt.publish_to "Persistence/test_a", testHash.to_json, retain: true;
		@persistence.setup(:test_a);

		sleep 0.3

		assert_equal testHash, @persistence[:test_a]
	end

	def test_callbacks
		testHash = {test: "Data"}
		@mqtt.publish_to "Persistence/test_a", testHash.to_json, retain: true;

		sleep 0.3

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

		sleep 0.3

		assert_equal newTestHash, @persistence[:test_a];

		assert_equal newTestHash, @setNewData;
		assert_equal newTestHash, @changeNewData;

		assert_equal testHash, @setOldData;
		assert_equal testHash, @changeOldData;
	end

	def test_nil()
		retainedValue = nil;
		@mqtt.subscribe_to "Persistence/test_nil" do |data|
			retainedValue = data;
		end

		@persistence.setup(:test_nil);
		@persistence[:test_nil] = "Not nil!";
		sleep 0.1
		@mqtt.publish_to "Persistence/test_nil", nil.to_json;
		sleep 0.3

		assert_nil @persistence[:test_nil];

		@mqtt.publish_to "Persistence/test_nil", "Not nil!", retain: true;
		sleep 0.3
		@persistence[:test_nil] = nil;
		sleep 0.1

		assert_equal nil.to_json, retainedValue;
	end

	def test_conversion_time
		retainedValue = nil;
		@mqtt.subscribe_to "Persistence/test_a" do |data|
			retainedValue = data;
		end
		testTime = Time.at(rand(0..99999999));

		@persistence.setup(:test_a, Time);
		@persistence[:test_a] = testTime;
		sleep 0.2

		assert_equal testTime.to_f.to_json, retainedValue;

		@mqtt.publish_to "Persistence/test_a", testTime.to_f.to_json
		sleep 0.3

		assert_equal testTime, @persistence[:test_a]
	end

	def test_custom_conversion
		retainedValue = nil;
		@mqtt.subscribe_to "Persistence/test_custom" do |data, tSplit|
			retainedValue = data;
		end

		customClass = Class.new do
			attr_accessor :firstValue, :secondValue

			def self.from_mqtt_string(data)
				return new().update_from_mqtt(data);
			end

			def update_from_mqtt(data)
				data = JSON.parse(data);
				return unless data;
				@firstValue = data["first"];
				@secondValue = data["second"];

				return self;
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

		sleep 0.2

		assert_equal cInstance.to_mqtt_string,
			retainedValue;

		@mqtt.publish_to "Persistence/test_custom", {
			first: "Test 1"
		}.to_json
		sleep 0.2

		cInstance = @persistence[:test_custom];

		assert_equal "Test 1", cInstance.firstValue
		assert_nil   cInstance.secondValue
	end
end
