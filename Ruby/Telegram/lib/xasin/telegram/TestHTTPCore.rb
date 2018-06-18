
require_relative 'HTTPCore.rb'

module Xasin
	module Telegram
		class TestCore < HTTPCore
			attr_accessor :lastPostData, :lastPostRequest
			attr_reader   :currentMessageID
			attr_accessor :toReturn

			def initialize()
			end

			def reconfigure()
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

			def attach_receptor(receptor)
			end

			def perform_post(postRequest, postData)
				@lastPostRequest << postRequest;
				@lastPostData << postData;

				return @toReturn;
			end
		end
	end
end
