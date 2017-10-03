require 'timeout'

module Xasin
	class Waitpoint
		def initialize()
			@waitThreads = Array.new();
			@fireID = 0;
			@lastArgument = Array.new();
		end

		def fire(args)
			@fireID += 1
			@lastArgument = args;
			@waitThreads.each do |t|
				t.run();
			end
		end

		def wait(allow_run: false)
			pausedID = @fireID;
			@waitThreads << Thread.current
			if(allow_run) then
				Thread.stop();
			else
				Thread.stop() until pausedID != @fireID;
			end
			@waitThreads.delete(Thread.current);

			return @lastArgument;
		end

		def wait_timeout(seconds = 1, allow_run: false)
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
