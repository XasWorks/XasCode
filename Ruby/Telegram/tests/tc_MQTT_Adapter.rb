
require_relative 'setup.rb'
require_relative 'tc_GroupingAdapter.rb'
require_relative '../lib/xasin/telegram/MQTT_Adapter.rb'


require 'mqtt/sub_testing'

$mqtt = MQTT::Testing::SubHandler.new();
$mqttTelegram = Xasin::Telegram::MQTT::Server.new($core, $mqtt);

# This class tests the MQTT Server class to ensure it has
# all of Grouping Adapter's functionality still present
class MQTT_Grouping_Test < Grouping_Test
	def setup
		$mqtt.prepare
		$core.prepare

		$mqttTelegram._reset();

		$adapter = $mqttTelegram;
	end
end

# This class tests whether or not the MQTT Server class
# successfully translates Grouping Adapter to MQTT signals and back
class MQTT_Server_Test < MiniTest::Test
	def setup
		$mqtt.prepare
		$core.prepare

		$mqttTelegram._reset();
	end

	def test_send
		uID = rand(0..9999);

		expected = {
			text: "Test Text",
			chat_id: uID,
			parse_mode: "Markdown"
		}

		# Test basic text sending
		$mqtt.publish_to "Telegram/#{uID}/Send", "Test Text"
		$mqtt.process_all
		assert_equal "sendMessage", $core.lastPostRequest.shift
		assert_equal expected, $core.lastPostData.shift

		# Test proper Username resolving
		$mqttTelegram.usernameList["TestUser"] = uID;

		$mqtt.publish_to "Telegram/TestUser/Send", "Test Text"
		$mqtt.process_all
		assert_equal "sendMessage", $core.lastPostRequest.shift
		assert_equal expected, $core.lastPostData.shift

		# Test if data hash is properly forwarded
		expected = {
			text: "Test Text",
			chat_id: uID,
			parse_mode: "Markdown",
			disable_notification: true
		}
		$mqtt.publish_to "Telegram/TestUser/Send", {
			text: "Test Text",
			silent: true
		}.to_json
		$mqtt.process_all
		assert_equal "sendMessage", $core.lastPostRequest.shift
		assert_equal expected, $core.lastPostData.shift
	end
end
