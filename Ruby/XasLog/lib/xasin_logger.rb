
require 'logger'

module XasLogger
	def XasLogger.init_formatting_logger(log_dest)
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

	def XasLogger.init_logger_list(list)
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

	def XasLogger.loggers()
		@loggers ||= [init_formatting_logger(STDOUT)];

		return @loggers
	end

	def XasLogger.loggers=(loggerList)
		@loggers = init_logger_list(loggerList);
	end

	module Mix
		def init_x_log(tag, logger = nil)
			@tag = tag;
			@log_level = ::Logger::INFO;

			if(logger.nil?)
				@log_dest = XasLogger.loggers();
			else
				@log_dest = XasLogger.init_logger_list(logger);
			end
		end

		def x_log(level, text)
			return if(@log_level > level)

			@log_dest.each do |logger|
				logger.add(level, "#{@tag}: #{text}");
			end
		end

		def x_logd(text)
			x_log(Logger::DEBUG, text);
		end
		def x_logi(text)
			x_log(Logger::INFO, text);
		end
		def x_logw(text)
			x_log(Logger::WARN, text);
		end
		def x_loge(text)
			x_log(Logger::ERROR, text);
		end
		def x_logf(text)
			x_log(Logger::FATAL, text);
		end

		def log_dest=(newDests)
			@log_dest = self.class.init_logger_list(newDests);
		end

		def log_level=(nLevel)
			@log_level = nLevel;
		end
	end

	class Standalone
		include XasLogger::Mix

		def initialize(tag, logger = nil)
			init_x_log(tag, logger);
		end
	end
end
