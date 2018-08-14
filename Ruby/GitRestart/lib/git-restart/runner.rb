
require 'mqtt/sub_handler'

require 'git'
require 'octokit'

require_relative "task.rb"

module GitRestart
	class Runner
		attr_accessor :name

		attr_accessor :repo, :branches, :exclude_branches, :start_on

		attr_reader	  	:next_tasks
		attr_reader		:current_task_file

		attr_reader		:mqtt
		attr_accessor	:octokit

		def current_commit()
			@git.object("HEAD").sha;
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
				begin
					data = JSON.parse(data, symbolize_names: true);
				rescue
					next;
				end

				next unless data[:branch];
				if(not @branches.empty?)
					next unless @branches.include? data[:branch];
				elsif(not @exclude_branches.empty?)
					next if @exclude_branches.include? data[:branch];
				end

				@branchQueue << data;
			end

			autostart();
			_start_task_thread();
		end

		def update_status(name, newStatus, message = nil)
			puts "Task #{@name} assumed a new status: #{newStatus}#{message ? " MSG:#{message}" : ""}"

			return unless @octokit;

			begin
			@octokit.create_status(@repo, current_commit(), newStatus, {
					context: "#{@name}/#{name}".gsub(" ", "_"),
					description: message,
				})
			rescue
			end
		end

		def _start_task_thread()
			@taskThread = Thread.new do
				loop do
					newData = @branchQueue.pop;

					@current_modified = newData[:touched];
					_switch_to(newData[:branch], newData[:head_commit]);
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
			taskFiles.split("\n").each do |t|
				puts "Looking at: #{t}"
				t.gsub!(/^\.\//,"");
				@current_task_file = t;

				begin
					load(t);
				rescue ScriptError, StandardError
					update_status("File #{t}", :failure, "File could not be parsed!")
					puts("File #{t} could not be loaded!");
				rescue GitRestart::TaskValidityError
					update_status("File #{t}", :failure, "Task-file not configured properly!")
					puts("Task-File #{t} is not configured properly!");
				end
			end

			puts "Finished loading! Next tasks: #{@next_tasks.keys}"
		end

		def _start_next_tasks()
			_generate_next_tasks();

			puts "\nStarting next tasks!"
			@next_tasks.each do |name, t|
				next unless t.active;
				next unless t.triggered?

				t.start();
				@current_tasks[name] = t;
			end
		end

		def _switch_to(branch, commit = nil)
			puts "\n\nSwitching to branch: #{branch}#{commit ? ",commit: #{commit}" : ""}"

			begin
				@git.fetch();
			rescue
			end

			if(branch != current_branch())
				_stop_all_tasks();
			else
				_stop_triggered_tasks();
			end
			@git.checkout(branch);
			@git.merge("origin/#{branch}");

			@git.reset_hard(commit);

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
