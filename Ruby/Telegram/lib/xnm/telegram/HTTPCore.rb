
require 'net/http'
require 'json'
require 'shellwords'

module XNM
module Telegram
	# This class handles the direct connection to the Telegram API
	# All it does is handle a receive loop with the fitting update IDs, as well
	# as a rescue'd "perform post" message.
	class HTTPCore
		def initialize(apikey)
			@apikey = apikey;

			@lastUpdateID = 0;
			# Start the receive loop. Shouldn't crash, but if it does we
			# want to know.
			@receiveThread = Thread.new do
				receive_loop();
			end
			@receiveThread.abort_on_exception = true

			# Receptors are class instances that will receive updates
			# from the HTTP update connection
			@receptors = Array.new();
		end

		private def _double_json_data(data)
			return {} unless data

			out_data = {}
			# JSON-Ify nested Hashes and Arrays to a String before the main
			# POST request is performed. Needed by Telegram, it seems.
			data.each do |key, val|
				if(val.is_a? Hash or val.is_a? Array)
					out_data[key] = val.to_json
				else
					out_data[key] = val;
				end
			end

			out_data
		end

		private def _raw_post(addr, data)
			d_str = Shellwords.escape(data.to_json)

			response = `curl -s -X POST -H "Content-Type: application/json" -d #{d_str} "#{addr}"`

			JSON.parse(response, symbolize_names: true)
		end
		# Perform a POST request.
		# @param method [String]  The Telegram bot API method that should be called
		# @param data [nil, Hash] The data to be sent with the command.
		#   Caution, any nested Hashes and Arrays will be converted to JSON
		#   BEFORE the Hash itself is also JSON-ified. Telegram apparently
		#   needs this to work.
		def perform_post(method, data = nil)
			call_address = "https://api.telegram.org/bot#{@apikey}/#{method}"

			# Rescue-construct to prevent a HTTP error from
			# crashing our system.
			timeoutLen = data[:timeout] if data.is_a? Hash
			timeoutLen ||= 4;
			retryCount = 0;
			begin
				Timeout.timeout(timeoutLen) do
					return _raw_post(call_address, _double_json_data(data));
				end
			rescue
				retryCount += 1;
				return {} if retryCount >= 3;

				sleep 0.5;
				retry
			end
		end

		def feed_receptors(data)
			# Hand it out to the receptors
			@receptors.each do |r|
				begin
					r.handle_packet(data);
				rescue => e
					warn "Error in repector: #{e}"
					warn e.backtrace.join("\n");
				end
			end
		end

		# Handle receiving of the data from Telegram API
		# This is done via the "getUpdates" HTTP Request, with a timeout
		# of 20s
		# Update-ID offset is handled automagically by the system, so you needn't
		# worry about it.
		def receive_loop()
			loop do
				begin
					# Perform the update request
					packet = perform_post("getUpdates", {timeout: 20, offset: @lastUpdateID + 1})

					next unless packet[:ok];
					# Check if there even was a message sent by Telegram
					# Due to the 20s timeout, a zero-length reply can happen
					next if packet[:result].length == 0;

					# Handle each result individually.
					packet[:result].each do |data|
						hUpdateID = data[:update_id].to_i
						# Calculate the maximum Update ID (for the offset "getUpdates" parameter)
						@lastUpdateID = [hUpdateID, @lastUpdateID].max

						feed_receptors data
					end
				rescue
					sleep 1
					retry
				end
			end
		end

		# TODO check if the class supports handle_packet
		def attach_receptor(receptorClass)
			@receptors << receptorClass;
		end
	end
end
end
