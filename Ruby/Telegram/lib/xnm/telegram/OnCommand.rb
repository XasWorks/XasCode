

module XNM
	module Telegram
		class OnCommand < OnTelegramEvent
			attr_accessor :deny_message
			attr_accessor :required_perms

			def initialize(options)
				super()

				@block = options[:block]
				@command = options[:command]

				@required_perms = [options[:permissions]].flatten.uniq

				@deny_message = options[:deny_msg] || 'You are not authorized, %s.'

				@priority += 5
			end

			def nomp_message(message)
				return if message.handled
				return unless message.command == @command

				if message.user.has_permissions? @required_perms
					@block.call message
				else
					message.reply @deny_message % [message.user.casual_name]
				end
			end
		end
	end
end
