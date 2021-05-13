
require_relative 'HTTPCore.rb'
require_relative 'Message.rb'
require_relative 'User.rb'

module XNM
	module Telegram
		# Dummy class to provide sensible defaults for most things.
		# Only really matters for priority, so that we can sort
		# after that.
		#
		# @todo Test if the sorting is the right way around
		class OnTelegramEvent
			attr_accessor :priority

			def initialize()
				@priority = 0
			end

			def nomp_message(message) end

			def <=>(other)
				self.priority <=> other.priority
			end
		end

		# Main handler class, provides interfaces.
		# It gives the user a way to send messages, set callbacks
		# and retrieve chats with detailed data.
		# More in-depth functions for messages are provided in the
		# message class itself.
		class Handler
			# Returns the {HTTPCore} used to communicate with
			# Telegram.
			attr_reader :core

			# List of permissions.
			# Set this to a Hash containing arrays to
			# included permissions, i.e. the following:
			# {
			#   "admin" => ["default", "advanced"]
			# }
			#
			# A user with "admin" would now also have "default" and "advanced"
			# permissions for command execution.
			#
			# @note The :sudo permission will always overwrite everything, use
			#   only for developer access!
			attr_accessor :permissions_list

			def self.from_options(options)
				out_handler = Handler.new(options['Key']);

				if perms = options['Permissions']
					raise ArgumentError, 'Permission list must be a hash!' unless perms.is_a? Hash
					out_handler.permissions_list = perms
				end

				if u_list = options['Users']
					raise ArgumentError, 'Userlist must be a hash!' unless u_list.is_a? Hash

					u_list.each do |key, extra_opts|
						u = out_handler[key];

						unless u
							warn "User #{key} could not be found."
							next
						end

						if u_perms = extra_opts['Permissions']
							u.add_permissions u_perms
						end
					end
				end

				out_handler
			end

			# initialize a new Handler.
			#
			# @param [String, HTTPCore] http_core The core to use, either
			#   a String representing the API Key, or an initialized Telegram Core
			def initialize(http_core)
				@core = if(http_core.is_a? Telegram::HTTPCore)
						http_core;
					elsif http_core.is_a? String
						Telegram::HTTPCore.new(http_core);
					else
						raise ArgumentError, "Could not make a valid HTTPCore from string!"
					end

				@core.attach_receptor(self);

				@chats = {}

				@on_telegram_event = []

				@permissions_list = {}
			end

			private def handle_message(message)
				message = Message.new(self, message);

				return unless message.valid

				(@on_telegram_event + message.chat&.on_telegram_event).sort.each do |evt|
					evt.nomp_message message

					break if message.handled
				end
			end

			# Handle an incoming callback query (inline keyboard button press)
			# as received from the HTTP Core
			private def handle_callback_query(cbq)
				# Generate a fake message to feed into the message
				# handling system ;)

				fake_msg = {
					from: cbq[:from],
					text: cbq[:data],
					chat: cbq[:message][:chat],
					message_id: cbq[:message][:message_id],
					reply_to_message: { message_id: cbq[:message][:message_id] }
				}

				handle_message fake_msg

				# Send out a callback query reply (i.e. Telegram now knows we saw it)
				@core.perform_post("answerCallbackQuery", {callback_query_id: cbq[:id]})
			end

			# Internal function, called from the Telegram Core.
			# This is meant to be fed a Hash representing one update object
			# from Telegram's /getUpdate function
			def handle_packet(packet)
				if m = packet[:message]
					handle_message m
				end

				if cbq = packet[:callback_query]
					handle_callback_query cbq
				end
			end

			# Return a {Chat} object directly constructed from the
			# passed Hash.
			#
			# Pass a Hash here to either fetch a chat with matching ID, or
			# else constuct a new chat from the provided data. Can be used
			# for getting a chat from a Message object, or adding a chat from
			# stored chat configs.
			def chat_from_object(object)
				chat_id = object[:id];

				if c = @chats[chat_id]
					return c
				end

				c = nil;
				if object[:username]
					c = User.new(self, object);
				else
					c = Chat.new(self, object);
				end

				@chats[chat_id] = c;

				c
			end

			# Try to find a chat with matching string ID.
			# Useful when trying to find a chat by username, or a channel.
			def chat_from_string(str)
				@chats.each do |_, chat|
					if chat.str_id == str
						return chat
					end
				end

				nil
			end

			# Return the {Chat} with given ID.
			# This will either return a known chat with the wanted ID, or else
			# will call getChat to fetch details on the wanted chat and
			# construct a new {Chat} object. May also return nil if the wanted
			# chat does not exist!
			def chat_from_id(num)
				if c = @chats[num]
					return c
				end

				chat_obj = @core.perform_post('getChat', { chat_id: num });

				return nil unless chat_obj[:ok]

				chat_from_object chat_obj[:result]
			end

			# Convenience function to get a chat by any means.
			# Pass a Number (interpreted as Chat ID), String (username)
			# or Hash into here to try and fetch a Chat based on the parameter.
			# Chat ID is preferred as it will let the system fetch the Chat from
			# Telegram's API.
			def get_chat(object)
				if object.is_a? Chat
					object
				elsif object.is_a? Hash
					chat_from_object object;
				elsif object.is_a? Numeric
					chat_from_id object
				elsif object.is_a? String
					if object =~ /^@/
						chat_from_id object
					else
						chat_from_string object
					end
				end
			end
			alias [] get_chat

			# Send a message to a given chat.
			# The following options are supported when sending:
			#
			# - silent: true/false, whether to enable or disable notification
			# - reply_to: {Message}/nil, try to reply to the given message ID.
			# - inline_keyboard: Hash or Array of Hashes, to set the inline keyboard
			#    with fitting commands.
			#
			# @note When a Inline Keyboard is used, the button presses are
			#   internally interpreted as messages. This way, they can feed into
			#   the /command syntax.
			def send_message(chat, text, **options)
				raise ArgumentError, "Text needs to be a string" unless text.is_a? String

				if text.length > 900
					text = text[0..900] + "..."
				end

				out_data = {
					chat_id: get_chat(chat).chat_id,
					parse_mode: 'HTML',
					text: text
				}

				if r = options[:reply_to]
					out_data[:reply_to_message_id] = r.to_i
				end

				if options[:silent]
					out_data[:disable_notification] = true;
				end

				if layout = options[:inline_keyboard]
					out_data[:reply_markup] = KeyboardLayout.new(layout).ilk_reply_markup
				end

				reply = @core.perform_post('sendMessage', out_data);

				Message.new(self, reply[:result]);
			end

			# Add a new callback on any generic message.
			# The provided block will be called with the Message as parameter
			# when something arrives. May additionally specify a RegExp to match
			# against, in which case the match is a second parameter to the
			# block.
			def on_message(regexp = nil, &block)
				raise ArgumentError, 'Block must be given!' unless block_given?

				out_evt = OnMessage.new({ block: block, regexp: regexp });
				@on_telegram_event << out_evt

				out_evt
			end

			# Add a new callback on any /command message.
			# The provided block will be called whenever the fitting /command
			# is called. Additionally, by setting a list of priorities
			# (options[:priorities] = []), only certain users may be allowed
			# to execute a command.
			#
			# The block will be passed the message as argument.
			def on_command(command, **options, &block)
				raise ArgumentError, 'Block must be given!' unless block_given?

				options[:block] = block
				options[:command] = command

				out_evt = OnCommand.new(options);
				@on_telegram_event << out_evt

				out_evt
			end
		end
	end
end

require_relative 'OnMessage.rb'
require_relative 'OnCommand.rb'
require_relative 'KeyboardLayout.rb'
