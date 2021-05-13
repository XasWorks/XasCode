
module XNM
	module Telegram
		class Message
			# Return whether or not parsing of the message was successful.
			# TODO Actually perfom validity checks.
			attr_reader :valid

			# Returns the message ID of this message.
			attr_reader :message_id
			# Returns the {Chat} object this message was sent in.
			attr_reader :chat
			# Returns the {User} Object that sent this message.
			attr_reader :user

			# Optional argument, ID of the message that was replied to.
			attr_reader :reply_to_id

			# String, text of the message.
			# Even if the message is not a String (i.e. a Sticker etc.),
			# this will be at least an empty string.
			attr_reader :text

			# Timestamp, from Telegram, that the message was sent on.
			attr_reader :timestamp

			# Optional, command included in this message.
			# Can be nil.
			attr_reader :command

			# Whether a command already handled this message.
			# Usually means that it should not be processed any further,
			# in order to prevent multiple commands from acting on the same
			# message and causing weird behaviors.
			attr_accessor :handled

			# Initialize a message object.
			# This will create a new message object with the given
			# Telegram "Message" Hash. It can be taken directly from
			# the Telegram API.
			#
			# The Message will automatically try to fetch the matching
			# {Chat} and {User} that sent the message, and will also
			# parse any additional metadata such as commands, stickers,
			# etc.
			def initialize(handler, message_object)
				@handler = handler

				return if message_object.nil?

				@valid = true;

				@message_id = message_object[:message_id]

				@chat = handler[message_object[:chat]]
				@user = handler[message_object[:from] || message_object[:sender_chat]]

				@reply_to_id = message_object.dig(:reply_to_message, :message_id)

				@text = message_object[:text] || "";

				@timestamp = Time.at(message_object[:date] || 0)

				m = /\/([\S]*)/.match(@text)
				@command = m[1] if m

				@handled = false
			end

			# Edit the text of the message.
			# Simple wrapper for the 'editMessageText' Telegram
			# API function, and will directly set the messge's text.
			#
			# parse_mode is set to HTML.
			def edit_text(text)
				out_data = {
					chat_id: @chat.chat_id,
					message_id: @message_id,
					parse_mode: 'HTML',
					text: text
				}

				@handler.core.perform_post('editMessageText', out_data);
			end
			alias text= edit_text

			# Try to delete this message.
			# Wrapper for Telegram's deleteMessage function
			def delete!
				@handler.core.perform_post('deleteMessage',
					{
						chat_id: @chat.chat_id,
						message_id: @message_id
					}
				);
			end

			# Send a text message with it's reply set to this.
			#
			# This will send a new message with given text, whose reply
			# message ID is set to this message. Makes it easy to respond to
			# certain events quite cleanly.
			def reply(text)
				@chat.send_message(text, reply_to: self)
			end

			# Send a message to the chat this message originated from.
			#
			# This is a wrapper for message.chat.send_message, as it allows
			# the Bot to easily respond to a {User}'s action in the same chat
			# the user wrote it in. It will simply forward all arguments to
			# {Chat#send_message}
			def send_message(text, **opts)
				@chat.send_message(text, **opts)
			end

			def to_s
				@text
			end

			def to_i
				@message_id
			end
		end
	end
end
