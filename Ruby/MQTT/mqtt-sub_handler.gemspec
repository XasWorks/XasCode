Gem::Specification.new do |s|
  s.name        = 'mqtt-sub_handler'
  s.version     = '0.0.1'
  s.date        = '2018-02-10'
  s.summary     = "Asynchronous, topic-based MQTT gem"
  s.description = "A gem to use when the normal ruby mqtt doesn't hit the spot."
  s.authors     = ["Xasin"]
  s.files       = [  "lib/mqtt/sub_handler.rb",
                     "lib/mqtt/subscription_classes.rb",
                     "lib/mqtt/Waitpoint.rb"]
  s.homepage    =
    'https://github.com/XasWorks/XasCode/tree/MQTT_GEM/Ruby/MQTT'
  s.license     = 'GPL-3.0'

  s.add_runtime_dependency "mqtt", ">= 0.5.0"
end
