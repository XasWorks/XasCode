
require 'logger'

module Xasin
	class XLogger < Logger
		attr_accessor :log_level

		def self.init_formatting_logger(log_dest)
			logger = Logger.new(log_dest);

			logger.formatter = proc do |severity, datetime, progname, msg|
				outStr = "[#{datetime.strftime("%b-%d %H:%M:%S.%L")}] #{msg}\n";

				case(severity)
				when 'DEBUG'
					outStr = "D #{outStr}";
				when 'WARN'
					outStr = "W #{outStr}".yellow
				when 'INFO'
					outStr = "I #{outStr}".green
				when 'ERROR'
					outStr = "E #{outStr}".red
				when 'FATAL'
					outStr = "F #{outStr}".black.on_red
				else
					outStr = "? #{outStr}".purple
				end

				outStr;
			end

			return logger;
		end

		def self.init_logger_list(list)
			lgrList = [list].flatten;

			outList = Array.new();
			lgrList.each do |logger|
				if(logger.is_a? Logger)
					outList << logger;
				else
					outList << init_formatting_logger(logger)
				end
			end
		end

		def self.loggers()
			@loggers ||= [init_formatting_logger(STDOUT)];

			return @loggers
		end

		def self.loggers=(loggerList)
			@loggers = init_logger_list(loggerList);
		end

		def initialize(tag, logger = nil)
			@tag = tag;
			@log_level = Logger::INFO;

			if(logger.nil?)
				@log_dest = self.class.loggers();
			else
				@log_dest = self.class.init_logger_list(logger);
			end
		end

		def add(level, text)
			return if(@log_level > level)

			@log_dest.each do |logger|
				logger.add(level, "#{@tag}: #{text}");
			end
		end

		def logd(text)
			add(Logger::DEBUG, text);
		end
		def logi(text)
			add(Logger::INFO, text);
		end
		def logw(text)
			add(Logger::WARN, text);
		end
		def loge(text)
			add(Logger::ERROR, text);
		end
		def logf(text)
			add(Logger::FATAL, text);
		end

		def log_dest=(newDests)
			@log_dest = self.class.init_logger_list(newDests);
		end
	end
end
