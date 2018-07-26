
module GitRestart
	class TaskValidityError < StandardError
	end

	class Task
		attr_reader 	:targets

		attr_accessor 	:signal
		attr_accessor	:expect_clean_exit
		attr_accessor	:report_status
		attr_accessor	:name, :status_file
		attr_accessor	:chdir

		attr_accessor	:active

		attr_reader		:lastStatus
		attr_reader		:status_message

		def self.branch=(newBranch)
			@branch = newBranch;
		end
		def self.branch()
			@branch;
		end
		def branch()
			self.class.branch();
		end

		def self.modified=(newAffected)
			@modified = newAffected;
		end
		def self.modified()
			return @modified;
		end
		def modified()
			self.class.modified
		end

		def watch(regEx)
			if(regEx.is_a? String)
				regEx = Regexp.quote(regEx);
			end

			if(@chdir)
				regEx = "#{Regexp.quote(@chdir)}/#{regEx.to_s}";
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
			watch(runner().current_task_file);

			@signal 	= "INT"
			@expect_clean_exit	= true;
			@exiting					= false;

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
					return true if f =~ regEx;
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
			print "Task #{@name} assumed a new status: #{status}#{message ? " MSG:#{message}" : ""}"
		end
		private :_report_status

		def start()
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
					@lastStatus = status.exitstatus();

					break unless @lastStatus == 0;
				end

				if(@lastStatus == 0)
					_report_status(:success);
				elsif(!@exiting || @expect_clean_exit)
					_report_status(:failure);
				end
			end
		end

		def stop()
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
