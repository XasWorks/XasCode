Gem::Specification.new do |s|
	s.name        = 'mqtt-sub_handler'
	s.version     = '0.1.6.10'
	s.date        = '2021-01-06'
	s.summary     = "Asynchronous, topic-based MQTT gem"
	s.description = "Asynchronous handling of callbacks that can be attached to individual topics, based on the mqtt gem."
	s.authors     = ["Xasin"]
	s.files       = [	"lib/mqtt/base_handler.rb",
						   "lib/mqtt/sub_handler.rb",
							"lib/mqtt/sub_testing.rb",
							"lib/mqtt/subscription_classes.rb",
							"lib/mqtt/Waitpoint.rb",
							"lib/mqtt/persistence.rb",
							"lib/mqtt/persistence_extensions.rb",
							"lib/mqtt/mqtt_hash.rb",
							"README.md"]
	s.homepage    =
	'https://github.com/XasWorks/XasCode/tree/master/Ruby/MQTT'
	s.license     = 'GPL-3.0'

	s.add_runtime_dependency "mqtt", ">= 0.5.0"
	s.add_runtime_dependency "json", "~> 2.2"
	s.add_runtime_dependency "xasin-logger", "~> 0.1"

	s.add_development_dependency "minitest"
	s.add_development_dependency "guard"
	s.add_development_dependency "guard-minitest"
end
