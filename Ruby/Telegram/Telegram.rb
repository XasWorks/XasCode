
require 'net/http'
require 'json'

module Telegram
	class HTTPCore
		def initialize(apikey)
			@apikey = apikey;

			@lastUpdateID = 0;
			Thread.new do
				receive_loop();
			end.abort_on_exception = true

			@receptors = Array.new();
		end

		def perform_post(method, data = nil)
			callAddress = URI "https://api.telegram.org/bot#{@apikey}/#{method}"

			begin
				if data
					response = Net::HTTP.post_form(callAddress, data);
				else
					response = Net::HTTP.get callAddress
				end

				response = JSON.parse(response.body, symbolize_names: true);
			rescue
				sleep 0.5;
				retry
			end

			return response;
		end

		def receive_loop()
			loop do
				begin
					packet = perform_post("getUpdates", {timeout: 20, offset: @lastUpdateID + 1})

					next unless packet[:ok];
					next if packet[:result].length == 0;

					puts "Handling packet: #{packet}"

					packet[:result].each do |data|
						hUpdateID = data[:update_id].to_i
						@lastUpdateID = [hUpdateID, @lastUpdateID].max

						@receptors.each do |r|
							r.handle_packet(data);
						end
					end
				rescue
					sleep 1
					retry
				end
			end
		end

		def attach_receptor(receptorClass)
			@receptors << receptorClass;
		end
	end

	class SingleUser
		def initialize(userChat, httpCore)
			@httpCore = httpCore.is_a?(Telegram::HTTPCore) ? httpCore : HTTPCore.new(httpCore);
			@httpCore.attach_receptor(self);

			@userChat = userChat;

			@message_procs 		= Array.new();
			@inlinebutton_procs	= Array.new();
		end

		def handle_packet(packet)
			if(packet[:message]) then
				return unless packet[:message][:chat][:id] == @userChat;

				@message_procs.each do |cb| cb.call(packet[:message]); end
				packet[:has_been_handled] = true;
			end

			if(packet[:callback_query]) then
				return unless packet[:callback_query][:message][:chat][:id] == @userChat;

				@inlinebutton_procs.each do |cb| cb.call(packet["callback_query"]); end
				packet[:has_been_handled] = true;
			end
		end

		def send_message(text, **args)
			args ||= Hash.new();
			args[:text] = text;

			args[:chat_id] 	=   @userChat;
			args[:parse_mode]	||= "Markdown";

			sent_message = @httpCore.perform_post("sendMessage", args);

			return sent_message[:result][:message_id];
		end

		def on_message(&block)
			@message_procs << block;
		end

		def on_inlinebutton_press(&block)
			@inlinebutton_procs << block;
		end
	end
end
