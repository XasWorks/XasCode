
require_relative "HTTPCore.rb"

module XNM
module Telegram
	class SingleUser
		attr_reader :httpCore

		def initialize(userChat, httpCore)
			# Check if we already have a HTTPCore, else create one
			@httpCore = if(httpCore.is_a? Telegram::HTTPCore)
					httpCore;
				else
					Telegram::HTTPCore.new(httpCore);
				end
			@httpCore.attach_receptor(self);
			@httpCore.attach_receptor(self);

			@userID = userChat;

			@message_procs 		= Array.new();
			@inlinebutton_procs	= Array.new();
		end

		def handle_packet(packet)
			if(packet[:message])
				return unless packet[:message][:chat][:id] == @userID;

				@message_procs.each do |cb| cb.call(packet[:message]); end
				packet[:has_been_handled] = true;
			end

			return unless packet[:callback_query]
			return unless packet[:callback_query][:message][:chat][:id] == @userID;

			@inlinebutton_procs.each { |cb| cb.call(packet[:callback_query]); }
			packet[:has_been_handled] = true;
		end

		def send_message(text, **args)
			args ||= Hash.new();
			args[:text] = text;

			args[:chat_id] 	=   @userID;
			args[:parse_mode]	||= "Markdown";

			sent_message = @httpCore.perform_post("sendMessage", args);

			return sent_message[:result][:message_id];
		end

		def edit_message(mID, text=nil, **args)
			args[:chat_id] 		= @userID;
			args[:message_id] 	= mID;

			args[:parse_mode]		||= "Markdown";

			if(text) then
				args[:text] = text;
				@httpCore.perform_post("editMessageText", args);
			else
				@httpCore.perform_post("editMessageReplyMarkup", args);
			end
		end

		def delete_message(mID)
			@httpCore.perform_post("deleteMessage", {chat_id: @userID, message_id: mID});
		end

		def on_message(&block)
			@message_procs << block;
		end
		def on_inlinebutton_press(&block)
			@inlinebutton_procs << block;
		end
	end
end
end
