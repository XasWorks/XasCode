require 'timeout'

module Xasin
	class Waitpoint
		attr_reader :lastArgument

		def initialize()
			@waitThreads = Array.new();
			@fireID = 0;
			@lastArgument = Array.new();
		end

		def fire(args = nil)
			@fireID += 1
			@lastArgument = args;
			@waitThreads.each do |t|
				t.run();
			end
		end

		def wait(seconds = nil, allow_run: false)
			pausedID = @fireID;
			@waitThreads << Thread.current

			timed_out = false;

			begin
				Timeout::timeout(seconds) {
					if(allow_run) then
						Thread.stop();
					else
						Thread.stop() until pausedID != @fireID;
					end
				}
			rescue Timeout::Error
				timed_out = true;
			ensure
				@waitThreads.delete(Thread.current);
			end

			return timed_out, @lastArgument;
		end
	end
end
