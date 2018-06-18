
require 'mqtt/sub_testing.rb'
require_relative '../../lib/xasin/telegram/TestHTTPCore.rb'

$mqtt = MQTT::Testing::SubHandler.new();
$core = Xasin::Telegram::TestCore.new();
