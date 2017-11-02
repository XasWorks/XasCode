
class Numeric
	def minutes
		return self * 60;
	end
	def hours
		return self * 60.minutes;
	end

	def days
		return self * 24.hours;
	end
end

class Time
	def self.today(extra = 0)
		cTimeA = Time.now.to_a;
		3.times do |i| cTimeA[i] = 0; end
		rTime = Time.local(*cTimeA);
		return (rTime += extra)
	end

	def self.today?(t)
		t = Time.at(t);
		return t.between? self.today(), self.today(24.hours);
	end
end
