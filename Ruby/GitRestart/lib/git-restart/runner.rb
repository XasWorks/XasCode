
require 'mqtt/sub_handler'
require 'git'

require_relative "task.rb"

module GitRestart
	class Runner
		attr_accessor :name

		attr_accessor :repo, :branches, :exclude_branches, :start_on

		attr_reader	  	:next_tasks
		attr_reader		:current_task_file

		def current_commit()
			@git.object("HEAD^").sha;
		end
		def current_branch()
			@git.current_branch();
		end
		def current_modified()
			@current_modified;
		end

		def initialize()
			GitRestart::Task.runner = self;

			@current_tasks = Hash.new();
			@next_tasks		= Hash.new();

			@branches 		= Array.new();
			@exclude_branches = Array.new();

			@branchQueue	= Queue.new();

			@git = Git.open(".");

			yield(self);

			@listenedSub = @mqtt.subscribe_to "GitHub/#{@repo}" do |data|
				puts "Received data: #{data}"
				begin
					data = JSON.parse(data, symbolize_names: true);
				rescue
					next;
				end

				puts "Processing data #{data}"

				next unless data[:branch];
				if(not @branches.empty?)
					next unless @branches.include? data[:branch];
				elsif(not @exclude_branches.empty?)
					next if @exclude_branches.include? data[:branch];
				end

				puts "Queueing data!"

				@branchQueue << data;
			end

			autostart();
			_start_task_thread();
		end

		def _start_task_thread()
			@taskThread = Thread.new do
				loop do
					newData = @branchQueue.pop;

					puts "Popped data: #{newData}"

					@current_modified = newData[:touched];
					_switch_to(newData[:branch], newData[:commit]);
				end
			end.abort_on_exception = true;
		end

		def _stop_tasks(taskList)
			taskList.each do |name, t|
				t.stop();
			end
			taskList.each do |name, t|
				t.join();
				@current_tasks.delete(name);
			end
		end
		def _stop_all_tasks()
			_stop_tasks(@current_tasks);
		end
		def _stop_triggered_tasks()
			_stop_tasks(@current_tasks.select {|k,v| v.triggered?});
		end

		def _generate_next_tasks()
			puts "Generating new tasks..."
			@next_tasks = Hash.new();

			taskFiles = `find ./ -nowarn -iname "*.gittask"`
			taskFiles.split("\n");

			taskFiles.each do |t|
				puts "Looking at: #{t}"
				t.gsub!(/^\.\//,"");
				@current_task_file = t;

				# TODO Add proper error reporting
				begin
					load(t);
				rescue LoadError
					puts("File #{t} could not be loaded!");
				rescue GitRestart::TaskValidityError
					puts("Task-File #{t} is not configured properly!");
				end
			end

			puts "Finished loading! Next tasks are: #{@next_tasks}"
		end

		def _start_next_tasks()
			_generate_next_tasks();

			puts "Starting next tasks!"
			@next_tasks.each do |name, t|
				next unless t.active;
				next unless t.triggered?

				t.start();
				@current_tasks[name] = t;
			end
		end

		def _switch_to(branch, commit = nil)
			puts "Switching to branch: #{branch}, commit: #{commit}"
			@git.fetch();

			if(branch != current_branch())
				_stop_all_tasks();
			else
				_stop_triggered_tasks();
			end
			@git.checkout(branch);
			@git.reset_hard(commit);

			@git.merge("origin/#{branch}");

			_start_next_tasks();
		end

		def autostart()
			return unless @start_on;
			@branchQueue << {branch: @start_on};
		end

		def mqtt=(mqtt)
			if(mqtt.is_a? String)
				@mqtt = MQTT::SubHandler.new(mqtt);
			else
				@mqtt = mqtt;
			end
		end
	end
end
