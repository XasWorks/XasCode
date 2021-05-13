Gem::Specification.new do |s|
  s.name        = 'xnm-telegram'
  s.version     = '0.3.0'
  s.date        = '2020-04-12'
  s.summary     = "XasinTheSystem's Telegram gem"
  s.description = "Multi-Purpose Telegram gem. Mainly for personal use, but also used as a Telegram to MQTT Bridge for IoT devices"
  s.authors     = ["Xasin", "Neira"]
  s.files       = [  Dir.glob('{bin,lib}/**/*'), 'README.md'].flatten
  s.homepage    =
    'https://github.com/XasWorks/XasCode/tree/GEM_Telegram/Ruby/Telegram'
  s.license     = 'GPL-3.0'

  s.add_runtime_dependency "mqtt-sub_handler"
end
