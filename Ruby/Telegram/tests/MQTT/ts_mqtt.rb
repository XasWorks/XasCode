
require_relative 'setup.rb'
require_relative '../../lib/xasin/telegram/MQTT_Adapter.rb'

$mqttTelegram = Xasin::Telegram::MQTT::Server.new($core, $mqtt);

require 'test/unit'

class MQTT_Server_Test < Test::Unit::TestCase
	def setup
		$mqtt.prepare
		$core.prepare
	end

	def test_keyboard_build
		assert_nil $mqttTelegram._process_inline_keyboard("Not valid!")

		expectedKeyboard = {
			inline_keyboard: [[
					{	text: "Key 1",
						callback_data: {i:"GID", k:"Key 1"}.to_json
					}]]
		}

		sentKeyboard = $mqttTelegram._process_inline_keyboard(["Key 1"], "GID");
		assert_equal expectedKeyboard.to_json, sentKeyboard.to_json,
			"Keyboard parsing did not equal expected result."

		sentKeyboard = $mqttTelegram._process_inline_keyboard([["Key 1"]], "GID");
		assert_equal expectedKeyboard.to_json, sentKeyboard.to_json,
			"Keyboard parsing did not equal expected result."


		expectedKeyboard = {
			inline_keyboard: [[
					{	text: "Key 1",
						callback_data: {i:"GID", k:"CB 1"}.to_json
					}]]
		}

		sentKeyboard = $mqttTelegram._process_inline_keyboard({"Key 1" => "CB 1"}, "GID");
		assert_equal expectedKeyboard.to_json, sentKeyboard.to_json,
			"Keyboard parsing did not equal expected result."

		sentKeyboard = $mqttTelegram._process_inline_keyboard([{"Key 1" => "CB 1"}], "GID");
		assert_equal expectedKeyboard.to_json, sentKeyboard.to_json,
			"Keyboard parsing did not equal expected result."
	end
end
