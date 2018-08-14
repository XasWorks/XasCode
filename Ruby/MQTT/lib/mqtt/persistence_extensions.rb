
class Time
	def to_mqtt_string
		to_f().to_json()
	end

	def self.from_mqtt_string(string)
		data = JSON.parse(string);
		return nil unless data.is_a? Numeric;

		return self.at(data);
	end
end
