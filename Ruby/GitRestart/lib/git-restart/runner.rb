require 'mqtt/sub_handler'

require_relative "runner.rb"

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

			@mqtt.subscribe_to "GitHub/#{@repo}" do |data|
				begin
					data = JSON.parse(data, symbolize_names: true);
				rescue
					next;
				end

				next unless data[:branch];
				if(@branches)
					next unless @branches.include? data[:branch];
				elsif(@exclude_branches)
					next if @exclude_branches.include? data[:branch];
				end

				@branchQueue << data;
			end

			autostart();
		end

		def _stop_all_tasks()
			@current_tasks.each do |name, t|
				t.stop();
			end
			@current_tasks.each do |name, t|
				t.join();
			end
			@current_tasks.clear();
		end

		def _switch_to(branch, commit = nil)
			@git.fetch();

			if(branch != current_branch())
				_stop_all_tasks();
			end
			@git.checkout(branch);
			@git.reset_hard(commit) if commit;

			@git.merge("origin/#{start_on}");
		end

		def _generate_next_tasks()
			@next_tasks = Hash.new();

			taskFiles = `find . -iname -nowarn "*.gittask"`
			taskFiles.split("\n");

			taskFiles.each do |t|
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
		end

		def _stop_

		def autostart()
			return unless @start_on;

			_switch_to(@start_on);
			_generate_next_tasks();

			@next_tasks.each do |name, t|
				next unless t.active
				t.start();
				@current_tasks[name] = t;
			end
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
