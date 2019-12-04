Gem::Specification.new do |s|
	s.name        = 'xasin-logger'
	s.version     = '0.1'
	s.date        = '2019-10-29'
	s.summary     = "Mixin-Based logging system"
	s.description = "ESP-IDF like, colorized, mixin-capable logging. It's just prettier this way, really."
	s.authors     = ["Xasin"]
	s.files       = [	"lib/xasin_logger.rb",
							"README.md"]
	s.homepage    =
	'https://github.com/XasWorks/XasCode/tree/master/Ruby/XasLog'
	s.license     = 'GPL-3.0'

	s.add_runtime_dependency "colorize", "~> 0.8"
end
