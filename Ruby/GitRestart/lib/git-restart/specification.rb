
module GitRestart
	class Specification
		def initialize(&block)
			@target = "start.rb"
		end

		def valid?()
			unless File.exist?(@target)
				raise ArgumentError, "No executable found!"
			end

			unless @mqtt
				raise ArgumentError, "No mqtt URL specified"
			end

			unless @signal
				
			end
		end
	end
end
