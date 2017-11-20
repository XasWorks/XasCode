
require 'json'

module Planetside2
	class Client
		def pull_data(verb: "get", collection: nil, parameters: [])
			raise ArgumentError, "Verb can only be get or count!" unless verb == "get" || verb == "count"

			getCommand = "http://census.daybreakgames.com/s:#{@apikey}/json/#{verb}/ps2:v2/"
			if collection
				getCommand += collection + "/";

			unless parameters.empty?
				raise ArgumentError, "Selection parameters need to be in pairs!" if (parameters.length % 2) != 0

				firstParameter = true;
				(0...(parameters.length)).step(2) do |i|
					if(firstParameter) then
						getCommand += "?";
					else
						getCommand += "&";
					end
					firstParameter = false;

					getCommand += "#{parameters[i]}=#{parameters[i+1]}";
				end
			end
			end

			begin
				return JSON.parse(`wget -q -O - "#{getCommand}"`);
			rescue
				return nil;
			end
		end

		def get_character_id(name)
			begin
				name.downcase!
				return @character_id_cache[name] if @character_id_cache.key? name

				retData = pull_data(
					collection: "character_name",
					parameters: [	"name.first_lower", name,
									"c:show", "character_id"]);

				return nil unless retData
				return nil if retData["returned"] == 0;

				@character_id_cache[name] = retData["character_name_list"][0]["character_id"].to_i;
				return @character_id_cache[name];
			rescue
				return nil;
			end
		end

		def get_online_status(id)
			begin
				id = get_character_id(id) if(id.is_a? String);
				return false unless id;

				retData = pull_data(
				collection: "characters_online_status",
				parameters: [	"character_id", id,
								"c:show", "online_status"]
				);

				return false 	unless retData;
				return false	if retData["returned"] == 0;
				return retData["characters_online_status_list"][0]["online_status"].to_i != 0;
			rescue
				return false;
			end
		end

		def get_character_world(id)
			id = get_character_id(id) if(id.is_a? String);
			return nil unless id;
			return @character_world_cache[id] if @character_world_cache.key? id;

			retData = pull_data(
			collection: "characters_world",
			parameters: [	"character_id", id,
							"c:show", "world_id"]
			);

			return nil if retData["returned"] == 0;
			return retData["characters_world_list"][0]["world_id"].to_i;
		end

		def initialize(apiKey = "example")
			@apikey = apiKey;

			@character_id_cache 	= Hash.new();
			@character_world_cache	= Hash.new();
		end
	end
end
