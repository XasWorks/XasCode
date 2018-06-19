
require_relative 'setup.rb'
require_relative '../../lib/xasin/telegram/MQTT_Adapter.rb'

require 'test/unit'


$mqttTelegram = Xasin::Telegram::MQTT::Server.new($core, $mqtt);

class MQTT_Server_Test < Test::Unit::TestCase
	def setup
		$mqtt.prepare
		$core.prepare

		$mqttTelegram._reset();
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
		assert_equal expectedKeyboard, sentKeyboard,
			"Keyboard parsing did not equal expected result."

		sentKeyboard = $mqttTelegram._process_inline_keyboard([["Key 1"]], "GID");
		assert_equal expectedKeyboard, sentKeyboard,
			"Keyboard parsing did not equal expected result."


		expectedKeyboard = {
			inline_keyboard: [[
					{	text: "Key 1",
						callback_data: {i:"GID", k:"CB 1"}.to_json
					}]]
		}

		sentKeyboard = $mqttTelegram._process_inline_keyboard({"Key 1" => "CB 1"}, "GID");
		assert_equal expectedKeyboard, sentKeyboard,
			"Keyboard parsing did not equal expected result."

		sentKeyboard = $mqttTelegram._process_inline_keyboard([{"Key 1" => "CB 1"}], "GID");
		assert_equal expectedKeyboard, sentKeyboard,
			"Keyboard parsing did not equal expected result."
	end

	def test_basic_send()
		chatID = rand(0..9999);
		expectedMessage = {
			chat_id: chatID,
			parse_mode: "Markdown",
			text: "Test Text"
		}

		# Assert text-only send, nothing more
		$mqttTelegram._handle_send({text: "Test Text"}, chatID);
		assert_equal "sendMessage", $core.lastPostRequest.shift
		assert_equal expectedMessage, $core.lastPostData.shift

		# Assert username resolving
		$mqttTelegram.usernameList["TestUser"] = chatID;

		$mqttTelegram._handle_send({text: "Test Text"}, "TestUser");
		assert_equal "sendMessage", $core.lastPostRequest.shift
		assert_equal expectedMessage, $core.lastPostData.shift

		# Assert notification disable
		expectedMessage[:disable_notification] = true;

		$mqttTelegram._handle_send({text: "Test Text", silent: true}, "TestUser");
		assert_equal "sendMessage", $core.lastPostRequest.shift
		assert_equal expectedMessage, $core.lastPostData.shift
	end
end
