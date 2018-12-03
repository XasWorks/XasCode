
require_relative 'HTTPCore.rb'

module Xasin
	module Telegram
		class TestCore < HTTPCore
			attr_accessor :lastPostData, :lastPostRequest
			attr_reader   :currentMessageID
			attr_accessor :toReturn

			def initialize()
				prepare();

				@receptors = Array.new();
			end

			def prepare()
				@lastPostData  	= Array.new();
				@lastPostRequest 	= Array.new();

				@currentMessageID = rand(0..9999);

				@toReturn = {
					ok: true,
					result: {
						message_id: @currentMessageID
					}
				}
			end

			def perform_post(postRequest, postData)
				@lastPostRequest << postRequest;
				@lastPostData << postData;

				return @toReturn;
			end

			def simulate_send_packet(packet)
				@receptors.each do |r|
					r.handle_packet(packet);
				end
			end

			def simulate_sent_message(text, chatID: "test", reply_id: nil )
				outData = Hash.new();

				outData[:chat] = {id: chatID};
				outData[:text] = text;

				if(reply_id)
					outData[:reply_to_message] = reply_id;
				end

				simulate_send_packet({message: outData});
			end
		end
	end
end
