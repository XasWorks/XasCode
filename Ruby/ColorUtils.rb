require 'interpolate'

class Color
	def self.RGB(r, g, b)
		self.new([r, g, b]);
	end

	def self.temperature(c, brightness = 1)
		c /= 100;
		if c <= 66 then
			r = 255;
		else
			r = 329.698727446 * ((c-60) ** -0.1332047592);
			r = 0 if r < 0;
			r = 255 if r > 255;
		end

		if c <= 66 then
			g = 99.4708025861 * Math.log(c) - 161.11956;
		else
			g = 288.1221695283 * ((c - 60) ** -0.0755148492);
		end
		g = 0 if g < 0;
		g = 255 if g > 255;

		if c >= 66 then
			b = 255;
		elsif c <= 19
			b = 0;
		else
			b = 138.5177312231 * Math.log((c-10)) - 305.0447927307;
		end
		b = 0 if b < 0;
		b = 255 if b > 255;

		self.new([r*brightness, g*brightness, b*brightness]);
	end
	def self.K(c, brightness = 1)
		self.temperature(c, brightness);
	end

	def self.daylight(brightness = nil, time: nil)
		time ||= Time.now();

		m = (time.sec()/60 + time.min())/60.0 + time.hour()

		colorTempPoints = {
			0   => 1800,
			0.2 => 1000,
			1   => 1000,
			5   => 1000,
			6.99 => 1800,
			7.25 => 4000,
			8    => 5000,
			15   => 5500,
			16.75 => 3000,
			17    => 2400,
			17.6  => 2400,
			18    => 5500,
			18.3  => 5000,
			18.5  => 3000,
			21    => 3000,
			22.5  => 2500,
			23    => 2000,
			24    => 1800,
		}
		bGraph = {
		0    => 0.4,
		0.2  => 0.2,
		6.7  => 0.3,
		7    => 0.1,
		7.25 => 1,
		20   => 1,
		22.5 => 0.7,
		23   => 0.5,
		24   => 0.4,
		}

		tempGraph = Interpolate::Points.new(colorTempPoints)
		temperature = tempGraph.at(m);
		brightGraph = Interpolate::Points.new(bGraph)
		brightness ||= brightGraph.at(m);
		self.temperature(temperature, brightness)
	end

	def self.from_s(s)
		cArray = Array.new();
		3.times do |i|
			cArray << s[(1+2*i)..(2+2*i)].to_i(16);
		end

		self.new(cArray);
	end

	def self.HSV(h, s = 1.0, v = 1.0)
		h = h%360;
		h += 360 if h < 0;

		c = v*s;
		x = c*(1.0 - ((h/60.0)%2 -1).abs);
		m = v - c;

		seg = (h/60).floor;

		rgb = Array.new();
		swap = (seg%2);

		rgb[(swap 	+ seg/2)%3] = c;
		rgb[(1-swap + seg/2)%3] = x;
		rgb[((2 + seg/2)%3)]		= 0;

		self.new([(rgb[0]+m) * 255, (rgb[1]+m) * 255, (rgb[2]+m) * 255])
	end

	def initialize(rgb)
		@scaling = 1.0;
		@rgb = rgb;
	end

	def rgb()
		oArray = Array.new();
		3.times do |i|
			oArray[i] = @rgb[i]*@scaling;
		end

		return oArray;
	end

	def interpolate(otherColor, balance)
		nArray = [0, 0, 0];
		3.times do |i|
			nArray[i] = self.rgb()[i]*(1 - balance) + otherColor.rgb()[i]*(balance);
		end

		return Color.RGB(*nArray);
	end

	def black?
		return @rgb.max == 0;
	end

	def white?
		return ((not black?) and (@rgb.min == @rgb.max))
	end

	def get_brightness()
		return @rgb.max() * @scaling;
	end

	def set_brightness(value)
		return self if @rgb.max == 0;

		value = [0, value].max;
		value = [255, value].min;

		@scaling = value.to_f/@rgb.max();

		self
	end

	def to_s
		"\#%02X%02X%02X" % rgb();
	end
end
