

module XNM
	module Telegram
		class OnMessage < OnTelegramEvent
			def initialize(options)
				super()

				@block = options[:block]
				@regexp = options[:regexp]

				@priority += 1 if @regexp
			end

			def nomp_message(message)
				if @regexp
					match = @regexp.match message.to_s

					if match
						@block.call message, match
						message.handled = true
					end
				else
					@block.call message
				end
			end
		end
	end
end
