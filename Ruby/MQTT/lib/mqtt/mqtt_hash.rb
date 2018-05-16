
require 'json'

require_relative 'sub_handler.rb'

module MQTT
	class TXHash
		attr_reader :hash

		def publish_layer(currentHash = nil, currentList = nil)
			currentHash ||= @hash;
			currentList ||= Array.new();

			status = catch(:halt) do
				loop do
					throw :halt, :publish_layer 			if currentList[-1] == "+"
					throw :halt, :publish_all_layered	if currentList[-1] == "#"
					throw :halt, :publish_hash 			if currentList.length == @topicList.length

					currentList << @topicList[currentList.length]
				end
			end

			case status
			when :publish_hash
				@mqtt.publish_to currentList.join("/"), currentHash.to_json, retain: true

			when :publish_layer
				if(currentHash.is_a? Hash) then
					currentHash.each do |key, val|
						newList = currentList.clone();
						newList[-1] = key;
						publish_layer(val, newList);
					end
				else
					@mqtt.publish_to currentList.join("/"), currentHash.to_json, retain: true
				end

			when :publish_all_layered
				if(currentHash.is_a? Hash) then
					currentHash.each do |key, val|
						publish_layer(val, (currentList.clone[-1] = key)<<"#");
					end
				else
					@mqtt.publish_to currentList.join("/"), currentHash.to_json, retain: true
				end
			end
		end
		alias update_hash publish_layer
		private :update_hash

		def initialize(mqtt, topic, startHash: Hash.new)
			@mqtt	 = mqtt;
			@topicList = MQTT::SubHandler.get_topic_split(topic);

			@hash  = startHash;
		end

		def hash=(newHash)
			@hash = newHash;
			update_hash
		end

		def [](key)
			return @hash[key]
		end

		def []=(key, val)
			@hash[key]= val
			update_hash
		end
	end
end
