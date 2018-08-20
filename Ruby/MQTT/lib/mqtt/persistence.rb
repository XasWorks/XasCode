
require 'json'
require_relative 'persistence_extensions.rb'

module MQTT
	class Persistence
		# Provide persistence for tasks that regularly restart, using MQTT retained messages.
		# As a fun side effect, this also allows for live monitoring and modifying!
		#
		# @author Xasin


		# Initialize the Persistence instance.
		#  This will immediately connect and subscribe to the wildcard of `dir + "/#"`
		# @param mqtt [MQTT::SubHandler] The MQTT instance to use for storing/communication
		# @param dir [String] A valid MQTT topic string under which to nest the persistence.
		def initialize(mqtt, dir = "Persistence")
			raise ArgumentError, "Not a mqtt class!" unless mqtt.is_a? MQTT::SubHandler

			@mqtt = mqtt;
			@dir  = dir;

			@rawData   = Hash.new();
			@paramList = Hash.new();

			@mqtt.subscribe_to "#{@dir}/+" do |data, key|
				_process_data(data, key[0].to_sym);
			end
		end


		# Prepare a piece of data to be sent to the MQTT topic
		# It must either respond to `to_mqtt_string` or to `to_json`
		def _prepare_send(data, key)
			if(data.respond_to? :to_mqtt_string)
				return data.to_mqtt_string;
			else
				return data.to_json
			end
		end
		private :_prepare_send

		# Parse a raw String received from MQTT.
		# If the given class type responds to `from_mqtt_string`, that
		# function is used for processing.
		# `JSON.parse` will be used otherwise.
		# @return [key-type, nil] Returns the parsed data, nil if that fails.
		def _parse_received(data, key)
			begin
				dType = @paramList[key][:type];
				if(dType.respond_to? :from_mqtt_string)
					return dType.from_mqtt_string(data);
				else
					return JSON.parse(data, symbolize_names: true);
				end
			rescue JSON::ParserError
				return nil;
			end
		end
		private :_parse_received

		# This processes new data received from MQTT.
		# It returns if the raw string hasn't changed, and will trigger the callbacks.
		def _process_data(data, key)
			return if @rawData[key] == data;
			@rawData[key] = data;

			return unless param = @paramList[key];

			oldData = param[:current];
			param[:current] = _parse_received(data, key);
			param[:cbList].each do |cb|
				cb.call(param[:current], oldData);
			end

			if(param[:first])
				param.delete :first
				param[:cbList] = [param[:cbList], param[:change_cb]].flatten;
			end
		end
		private :_process_data

		# Force setting up a key, overwriting what was already configured.
		# You usually don't need this!
		# @param key [Symbol] The key that should be set up.
		# @param type_class [#from_mqtt_string] The class to use to parse the data. A JSON-conversion
		#  is used if the class doesn't respond to `from_mqtt_string`
		def setup!(key, type_class = nil)
			raise ArgumentError, "Key needs to be a symbol!" unless key.is_a? Symbol

			@paramList[key] = {
				type: 	type_class,
				current: nil,
				cbList:	Array.new(),

				first:		true,
				change_cb:	Array.new(),
			}

			if(data = @rawData[key])
				@rawData[key] = nil;
				_process_data(data, key);
			end
		end
		# Set up a key, unless it has already been configured.
		# @param (See #setup!)
		def setup(key, type_class = nil)
			return if @paramList.key? key;
			setup!(key, type_class);
		end

		# Read out a given, set-up key, returning `nil` if nothing has been read yet (or the value is nil right now),
		# returning the data otherwise.
		# If a block is given (using the on_set alias), this block will be added as a callback.
		# @param key [Symbol] The key to fetch. Must have been set up first!
		# @yieldparam newData The newly parsed data (depending on the key's type.)
		# @yieldparam oldData The former data that was present.
		def [](key, &callback)
			raise ArgumentError, "Key needs to be a symbol!" unless key.is_a? Symbol
			raise ArgumentError, "Key has not been set up!" unless @paramList.key? key;

			if(callback)
				@paramList[key][:cbList] << callback
				yield(@paramList[key][:current], nil) if @paramList[key][:current]
			end

			return @paramList[key][:current];
		end
		alias on_set []

		# Set a given key to a new value, provided the key has been set up.
		# Will run the `on_change` and `on_set` callbacks immediately, then publish to MQTT.
		# @param key [Symbol] The key to write. Must have been set up first!
		# @param data [#to_json, #to_mqtt_string] The data to write.
		def []=(key, data)
			raise ArgumentError, "Key needs to be a symbol!" unless key.is_a? Symbol
			raise ArgumentError, "Key has not been set up!" unless param = @paramList[key];

			newString = _prepare_send(data, key);
			return if @rawData[key] == newString;
			@rawData[key] = newString;

			if(param[:first])
				param.delete :first
				param[:cbList] = [param[:cbList], param[:change_cb]].flatten;
			end

			oldData = param[:current];
			param[:current] = data;
			param[:cbList].each do |cb|
				cb.call(param[:current], oldData);
			end

			@mqtt.publish_to "#{@dir}/#{key}", newString, retain: true;
		end

		# Add a callback to be triggered whenever the key's value changes.
		# The slight difference to `on_set` is that this callback is not executed on the very first
		# value received from MQTT, making it easier to not trigger some code on restart, only on actual changes.
		# @param (See #[])
		# @yieldparam (See #[])
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
