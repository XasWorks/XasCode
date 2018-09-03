
# @author Xasin
module GitRestart
	# The Error-Class used to signal when a Task is set up wrong
	class TaskValidityError < StandardError
	end

	# This class is used to define "Tasks". Each task represents
	# a set of commands, which it executes in chronological order, or until
	# a task errors.
	# Additionally, it will kill execution of tasks with a specified kill-signal
	# when an update was detected from GitHub.
	# The failure-status of the tasks can also be reported via Octokit, allowing this
	# to be used as a simple CI or Test system for various languages.
	class Task
		# The array of tasks to execute. Each target will be executed in the given
		# order via `Process.spawn`.
		# @return [Array<String>]
		attr_reader 	:targets

		# The signal (as String, like Signal.list) to use to kill the process.
		# Can be nil to disable killing
		attr_accessor 	:signal
		# Whether or not to report failure if the currently running target
		# has a non-zero exit status after having been killed. Only makes sense
		# together with report_status
		attr_accessor	:expect_clean_exit
		# Whether or not to report failure/success status to GitHub using Octokit
		attr_accessor	:report_status
		# Defines this as a "CI_Task". Such a task will always run on an update,
		# regardless what files changed. Useful if you always want a status report
		# on GitHub.
		attr_accessor  :ci_task
		# Name of the Task. *Required*. Used as *unique* ID, and to report status to GitHub
		attr_accessor	:name
		# The file to use to retrieve a single-line status info for the "description"
		# string of the GitHub status. Only the last *non-indented* line is used,
		# which allows the output of Minitest to be used directly.
		attr_accessor  :status_file

		# Whether or not this task is active. Usually set via #on_branches,
		# but can be used to manually disable or enable this task based on
		# config files, ENV variables etc.
		attr_accessor	:active

		# The last status-code of this Task. Used internally.
		attr_reader		:lastStatus
		# The last status-message of this task. Used internally.
		attr_reader		:status_message

		# @api private
		def self.runner=(runner)
			@runner = runner;
		end
		# @api private
		def self.runner()
			return @runner;
		end
		# @return [GitRestart::Runner] Responsible Runner class
		def runner()
			return self.class.runner();
		end

		# @return [String] Name of the current branch
		def branch()
			runner().current_branch();
		end
		# @return [String] Full SHA of the current commit
		def current_commit()
			runner().current_commit();
		end

		# @return [Array<String>] A list of all files that were modified in the commit we are checking
		def modified()
			runner().current_modified();
		end

		# Use this function to specify which files trigger a restart for this Task
		# Files can be specified as a RegEx, and can be "local/like.this" or "/reference/from/project.root"
		def watch(regEx)
			if(regEx.is_a? String)
				regEx = Regexp.quote(regEx);
			end

			@watched << Regexp.new(regEx);
		end

		# Specify which branches to run on. Not needed if "active" is just set to true
		def on_branches(branches)
			[branches].flatten.each do |b|
				@activ |= (b == branch());
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

			watch(File.basename(runner().current_task_file));

			yield(self);

			valid?

			@status_file ||= "/tmp/TaskLog_#{@name}_#{current_commit()}";

			if(runner().next_tasks[@name])
				raise TaskValidityError, "A task of name #{@name} already exists!"
			else
				runner().next_tasks[@name] = self;
			end
		end

		def triggered?
			return true if modified().nil?
			return true if @ci_task

			@watched.each do |regEx|
				modified().each do |f|
					if regEx.to_s =~ /^\(\?\-mix:\\\/(.*)\)$/ then
						return true if f =~ Regexp.new($1);
					else
						next unless f =~ /#{Regexp.quote(@chdir)}(.*)/
						return true if $1 =~ regEx;
					end
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

		def _rm_logfile()
			if File.exist?("/tmp/TaskLog_#{@name}_#{current_commit()}") then
				File.delete("/tmp/TaskLog_#{@name}_#{current_commit()}");
			end
		end
		def _get_statusline()
			return "No status specified" unless File.exist? @status_file

			sMsg = ""
			File.open(@status_file, "r") do |sFile|
				sFile.each_line do |l|
					l.chomp!
					next if l == "";
					next if l =~ /^\s+/;

					sMsg = l;
				end
			end

			return sMsg;
		end

		def _report_status(status, message = nil)
			message ||= _get_statusline();
			@status_message = message;

			return unless @report_status

			runner().update_status(@name, status, message);
		end
		private :_report_status

		def start()
			puts "Starting Task: #{@name}"

			if @targets.empty?
				_report_status(:success, "No tasks to run!");
				return
			end

			@executionThread = Thread.new do
				_report_status(:pending);

				_rm_logfile();
				@targets.each do |target|
					@statuschange_mutex.synchronize {
						break if @exiting
						options = {
							[:out, :err] => "/tmp/TaskLog_#{@name}_#{current_commit()}"
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
					_rm_logfile();
				elsif(!@exiting || @expect_clean_exit)
					_report_status(:failure);
				end
			end
			@executionThread.abort_on_exception = true;

			sleep 0.01
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
