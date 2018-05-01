Gem::Specification.new do |s|
  s.name        = 'mqtt-sub_handler'
  s.version     = '0.0.4'
  s.date        = '2018-04-24'
  s.summary     = "Asynchronous, topic-based MQTT gem"
  s.description = "Asynchronous handling of callbacks that can be attached to individual topics, based on the mqtt gem."
  s.authors     = ["Xasin"]
  s.files       = [  "lib/mqtt/sub_handler.rb",
                     "lib/mqtt/subscription_classes.rb",
                     "lib/mqtt/Waitpoint.rb",
					 "README.md"]
  s.homepage    =
    'https://github.com/XasWorks/XasCode/tree/MQTT_GEM/Ruby/MQTT'
  s.license     = 'GPL-3.0'

  s.add_runtime_dependency "mqtt", ">= 0.5.0"
end
