
module XNM
	module Telegram
		class Chat
			attr_reader :chat_id
			attr_reader :str_id

			attr_reader :casual_name

			attr_reader :chat_obj

			attr_reader :on_telegram_event

			# Initialize a chat object.
			# Call this to generate a new chat object. It will
			# always have to be called with a Chat object, as returned by
			# a message's "chat" field or the getChat function
			def initialize(handler, chat_info)
				@handler = handler;

				@chat_id = chat_info[:id];
				@str_id  = chat_info[:username] ||
				           chat_info[:title]

				@casual_name = @str_id;

				@chat_obj = chat_info;

				@on_telegram_event = []
			end

			# Send a message to this chat.
			# @see Handler#send_message
			def send_message(text, **options)
				@handler.send_message(self, text, **options);
			end

			# Add a new message callback.
			# Similar to the Handler's function, but only applies
			# to this chat's messages. Especially nice for one-on-one bots.
			# @see Handler#on_message
			def on_message(regexp = nil, &block)
				raise ArgumentError, 'Block must be given!' unless block_given?

				out_evt = OnMessage.new({ block: block, regexp: regexp });
				@on_telegram_event << out_evt

				out_evt
			end

			# Add a command callback.
			# Equivalent to {Handler#on_command}, but will only be called
			# on commands issued in this chat.
			def on_command(command, **options, &block)
				raise ArgumentError, 'Block must be given!' unless block_given?

				options[:block] = block
				options[:command] = command

				out_evt = OnCommand.new(options);
				@on_telegram_event << out_evt

				out_evt
			end

			# Return a Telegram mention link.
			# Can be inserted into a Telegram HTML formatted message, and
			# allows people to click on the name.
			def tg_mention
				"<a href=\"tg://user?id=#{@chat_id}\">@#{@str_id}</a>"
			end

			# Return a more human-friendly name for the chat.
			def casual_name
				@str_id
			end

			def to_i
				@chat_id
			end
			
			def to_s
				@casual_name
			end
		end
	end
end
