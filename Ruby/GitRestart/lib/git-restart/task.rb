
module GitRestart
	class Task
		attr_reader 	:targets
		attr_accessor 	:signal
		attr_accessor	:restart_on_exit, :restart_on_failure, :expect_clean_exit
		attr_accessor	:complete

		def initialize()
			@targets = Array.new();
			@signal 	= "INT"

			@restart_on_exit 		= false;
			@restart_on_failure	= false;
			@expect_clean_exit	= true;

			@complete 	= true;
			@exiting		= false;

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

		def start()
			@executionThread = Thread.new do
				currentTargetI = 0;
				loop do
					target = @targets[currentTargetI];

					@currentPID = Process.spawn(target, [:in, :out, :err] => "/dev/null", chdir: @chdir);
					pid, status = Process.wait2(@currentPID);
					@lastStatus = status.exitstatus;

					success 	= @lastStatus == 0;
					lastTask = @targets[currentTargetI + 1].nil?

				end
			end
		end

		def stop()
		end

		def join()
		end
	end
end
