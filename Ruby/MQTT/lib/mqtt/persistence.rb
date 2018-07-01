
require 'json'

module MQTT
	class Persistence


		def initialize(mqtt, dir = "Persistence")
			raise ArgumentError, "Not a mqtt class!" unless mqtt.is_a? MQTT::SubHandler

			@mqtt = mqtt;
			@dir  = dir;

			@paramList = Hash.new();
		end

		def _prepare_send(data, key)
			case @paramList[key][:type]
			when Time
				return nil.to_json unless data;
				return data.to_i.to_json;
			else
				return data.to_json
			end
		end
		def _parse_received(data, key)
			begin
				case @paramList[key][:type]
				when Time
					i = JSON.parse(data);
					return nil unless i.is_a? Numeric;
					return Time.at(i);
				else
					return JSON.parse(data);
				end
			rescue JSON::ParserError
				return nil;
			end
		end

		def _process_data(data, key)
			return unless param = @paramList[key];
			return if param[:current_raw] == data;

			oldData = param[:current];
			param[:current] = _parse_received(data, key);
			param[:cbList].each do |cb|
				cb.call(param[:current], oldData);
			end

			if(param[:first])
				param.delete :first
				param.cbList = [param[:cbList], param[:change_cb]].flatten;
			end
		end

		def setup!(key, type_class = nil)
			raise ArgumentError, "Key needs to be a symbol!" unless key.is_a? Symbol

			@paramList[key] = {
				type: 	type_class,
				current_raw: nil,
				current: nil,
				cbList:	Array.new(),

				first:		true,
				change_cb:	Array.new(),
			}
		end
		def setup(key, type_class = nil)
			return if @paramList.key? key;
			setup!(key, type_class);
		end

		def start()
			@mqtt.subscribe_to "#{@dir}/+" do |data, key|
				_process_data(data, key[0].to_sym);
			end
		end

		def [](key, &callback)
			raise ArgumentError, "Key needs to be a symbol!" unless key.is_a? Symbol
			raise ArgumentError, "Key has not been set up!" unless @paramList.key? key;

			if(callback)
				@paramList[key][:cbList] << callback
				callback.call(@paramList[:current], nil);
			end

			return @paramList[key][:current];
		end
		def []=(key, data)
			raise ArgumentError, "Key needs to be a symbol!" unless key.is_a? Symbol
			raise ArgumentError, "Key has not been set up!" unless param = @paramList[key];

			newString = _prepare_send(data, key);
			return if param[:current_raw] == newString;

			if(param[:first])
				param.delete :first
				param[:cbList] = [param[:cbList], param[:change_cb]].flatten;
			end

			oldData = param[:current];
			param[:current] = data;
			param.cbList.each do |cb|
				cb.call(param[:current], oldData);
			end
			param[:current_raw] = newString;

			@mqtt.publish_to "#{dir}/#{key}", newString, retain: true;
		end

		def on_change(key, &callback)
			raise ArgumentError, "Key needs to be a symbol!" unless key.is_a? Symbol
			raise ArgumentError, "Key has not been set up!" unless @paramList.key? key;

			raise ArgumentError, "A callback needs to be given!" unless callback;

			if(@paramList[key][:first])
				@paramList[key][:change_cb] << callback;
			else
				@paramList[key][:cbList] << callback;
			end
		end
	end
end
