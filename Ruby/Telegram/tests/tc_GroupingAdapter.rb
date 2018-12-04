
require_relative 'setup.rb'
require_relative '../lib/xasin/telegram/GroupingAdapter.rb'

$adapter = Xasin::Telegram::GroupingAdapter.new($core);

# This class tests all of GroupingAdapter's functions.
# That includes keyboard parsing and constructing, sending and
# receving of messages, etc.
class Grouping_Test < MiniTest::Test
	def setup()
		$core.prepare();
		$adapter._reset();
	end

	def test_keyboard_build
		assert_nil $adapter._process_inline_keyboard("Not valid!")

		expectedKeyboard = {
			inline_keyboard: [[
					{
						text: "Key 1",
						callback_data: {i: "GID",
											 k: "Key 1"}.to_json
					}]]
		}

		sentKeyboard = $adapter._process_inline_keyboard(["Key 1"], "GID");
		assert_equal expectedKeyboard, sentKeyboard,
						 "Keyboard parsing did not equal expected result."

		sentKeyboard = $adapter._process_inline_keyboard([["Key 1"]], "GID");
		assert_equal expectedKeyboard, sentKeyboard,
						 "Keyboard parsing did not equal expected result."


		expectedKeyboard = {
			inline_keyboard: [[
					{
						text: "Key 1",
						callback_data: {i:"GID", k:"CB 1"}.to_json
					}]]
		}

		sentKeyboard = $adapter._process_inline_keyboard({"Key 1" => "CB 1"}, "GID");
		assert_equal expectedKeyboard, sentKeyboard,
						 "Keyboard parsing did not equal expected result."

		sentKeyboard = $adapter._process_inline_keyboard([{"Key 1" => "CB 1"}], "GID");
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
		$adapter._handle_send({text: "Test Text"}, chatID);
		assert_equal "sendMessage", $core.lastPostRequest.shift
		assert_equal expectedMessage, $core.lastPostData.shift

		# Assert username resolving
		$adapter.usernameList["TestUser"] = chatID;

		$adapter._handle_send({text: "Test Text"}, "TestUser");
		assert_equal "sendMessage", $core.lastPostRequest.shift
		assert_equal expectedMessage, $core.lastPostData.shift

		# Assert notification disable
		expectedMessage[:disable_notification] = true;

		$adapter._handle_send({text: "Test Text", silent: true}, "TestUser");
		assert_equal "sendMessage", $core.lastPostRequest.shift
		assert_equal expectedMessage, $core.lastPostData.shift
	end

	def test_basic_receive
		$core.simulate_sent_message("Test text!");
		assert_equal "Test text!", $adapter.testLastData[:text];
	end
end
