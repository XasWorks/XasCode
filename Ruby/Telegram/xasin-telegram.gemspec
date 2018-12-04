Gem::Specification.new do |s|
  s.name        = 'xasin-telegram'
  s.version     = '0.2.4'
  s.date        = '2018-06-18'
  s.summary     = "Xasin's Telegram gem"
  s.description = "Multi-Purpose Telegram gem. Mainly for personal use, but also used as a Telegram to MQTT Bridge for IoT devices"
  s.authors     = ["Xasin"]
  s.files       = [  "lib/xasin/telegram/HTTPCore.rb",
							"lib/xasin/telegram/MQTT_Adapter.rb",
							"lib/xasin/telegram/SingleUser.rb",
							"lib/xasin/telegram.rb",
							"lib/xasin/telegram/TestHTTPCore.rb",
							"README.md"]
  s.homepage    =
    'https://github.com/XasWorks/XasCode/tree/GEM_Telegram/Ruby/Telegram'
  s.license     = 'GPL-3.0'

  s.add_runtime_dependency "mqtt-sub_handler"
end
