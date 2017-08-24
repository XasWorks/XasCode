

class Color
	attr_reader :k

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

	def initialize(values)
		@rgb = values;

		@scaling = 1;
	end

	def rgb()
		oArray = Array.new();
		3.times do |i|
			oArray[i] = @rgb[i]*@scaling;
		end

		return oArray;
	end

	def get_brightness(value)
		return @rgb.max * @scaling;
	end

	def set_brightness(value)
		value = [0, value].max;
		value = [255, value].min;

		@scaling = value/@rgb.max;

		rgb();
	end

	def to_s
		"\#%02X%02X%02X" % rgb();
	end
end
