
module GitRestart
	class Task
		attr_reader 	:targets

		attr_accessor 	:signal
		attr_accessor	:expect_clean_exit
		attr_accessor	:name, :status_descriptor

		def initialize()
			@targets = Array.new();
			@signal 	= "INT"

			@expect_clean_exit	= true;
			@exiting					= false;

			yield(self);

			@name ||= @targets[-1];
		end

		def valid?()
			@targets.each do |t|
				raise ArgumentError, "Target-File #{t} wasn't found!" unless File.exist?(t)
			end

			unless Signal.list[@signal] or @signal.nil?
				raise ArgumentError, "The specified kill-signal is not valid!"
			end
		end

		def report_error()

		end

		def start()
			@executionThread = Thread.new do
				@targets.each do |target|
					break if (@exiting && !@signal.nil?);

					@currentPID = Process.spawn(target, [:in, :out, :err] => "/dev/null", chdir: @chdir);
					nil, status = Process.wait2(@currentPID);
					@lastStatus = status.exitstatus();

					break unless @lastStatus == 0;
				end

				if(@lastStatus == 0)
					report_success();
				elsif(!@exiting || @expect_clean_exit)
					report_error();
				end
			end
		end

		def stop()
		end

		def join()
		end
	end
end
