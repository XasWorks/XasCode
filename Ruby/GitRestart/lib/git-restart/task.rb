
module GitRestart
	class TaskValidityError < StandardError
	end

	class Task
		attr_reader 	:targets

		attr_accessor 	:signal
		attr_accessor	:expect_clean_exit
		attr_accessor	:report_status
		attr_accessor	:name, :status_file

		attr_accessor	:active

		attr_reader		:lastStatus
		attr_reader		:status_message

		def self.runner=(runner)
			@runner = runner;
		end
		def self.runner()
			return @runner;
		end
		def runner()
			return self.class.runner();
		end

		def branch()
			runner().current_branch();
		end
		def modified()
			runner().current_modified();
		end

		def watch(regEx)
			if(regEx.is_a? String)
				regEx = Regexp.quote(regEx);
			end

			@watched << Regexp.new(regEx);
		end

		def on_branches(branches)
			[branches].flatten.each do |b|
				@active |= (b == branch());
			end
		end

		def initialize()
			@statuschange_mutex = Mutex.new();

			@targets = Array.new();
			@watched = Array.new();

			@signal 	= "INT"
			@expect_clean_exit	= true;
			@exiting					= false;

			@lastStatus = 0;
			@chdir = File.dirname(runner().current_task_file);

			watch(runner().current_task_file);

			yield(self);

			valid?

			if(runner().next_tasks[@name])
				raise TaskValidityError, "A task of name #{@name} already exists!"
			else
				runner().next_tasks[@name] = self;
			end
		end

		def triggered?
			return true if modified().nil?

			@watched.each do |regEx|
				modified().each do |f|
					next unless f =~ /#{Regexp.quote(@chdir)}(.*)/
					return true if $1 =~ regEx;
				end
			end

			return false;
		end

		def valid?()
			unless Signal.list[@signal] or @signal.nil?
				raise TaskValidityError, "The specified kill-signal is not valid!"
			end

			unless @name
				raise TaskValidityError, "A name needs to be set for identification!"
			end
		end

		def _report_status(status, message = nil)
			@status_message = ""

			return unless @report_status

			runner().update_status(@name, status, message);
			puts "Task #{@name} assumed a new status: #{status}#{message ? " MSG:#{message}" : ""}"
		end
		private :_report_status

		def start()
			puts "Starting Task: #{@name}"

			return if @targets.empty?
			sleep 0.01
			
			@executionThread = Thread.new do
				_report_status(:pending);

				@targets.each do |target|
					@statuschange_mutex.synchronize {
						break if @exiting
						options = {
							[:in, :out, :err] => "/dev/null"
						}
						options[:chdir] = @chdir if @chdir

						@currentPID = Process.spawn(target, options);
					}

					status = Process.wait2(@currentPID)[1];
					@currentPID = nil;
					@lastStatus = status.exitstatus();

					break unless @lastStatus == 0;
				end

				if(@lastStatus == 0)
					_report_status(:success);
				elsif(!@exiting || @expect_clean_exit)
					_report_status(:failure);
				end
			end
			@executionThread.abort_on_exception = true;
		end

		def stop()
			puts "Stopping Task: #{@name}"
			return if @signal.nil?

			@statuschange_mutex.synchronize {
				@exiting = true;
				if(p = @currentPID)
					Process.kill(@signal, p);
				end
			}
		end

		def join()
			@executionThread.join();
		end
	end
end
