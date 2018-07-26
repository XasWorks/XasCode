require 'mqtt/sub_handler'

require_relative "runner.rb"

module GitRestart
	class Runner
		attr_accessor :name

		attr_accessor :repo, :branches, :exclude_branches, :start_on

		def current_commit()
			@git.object("HEAD^").sha;
		end
		def current_branch()
			@git.current_branch();
		end

		def initialize()
			@currentTasks 	= Hash.new();
			@nextTasks		= Hash.new();

			@branches 		= Array.new();
			@exclude_branches = Array.new();

			@branchQueue	= Queue.new();

			yield(self);

			@git = Git.open(".");

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

		def autostart()
			return unless @start_on;

			@git.fetch();
			@git.checkout(@start_on);
			@git.merge("origin/#{@start_on}");
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
