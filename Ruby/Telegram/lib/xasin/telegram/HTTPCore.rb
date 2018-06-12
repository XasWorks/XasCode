
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
end
