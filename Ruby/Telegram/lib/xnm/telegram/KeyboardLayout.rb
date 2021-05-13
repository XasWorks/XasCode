
module XNM
	module Telegram
		class KeyboardLayout
			def initialize(start_config = nil)
				@rows = Array.new()

				if start_config.is_a? Hash
					start_config = [start_config]
				end

				if start_config.is_a? Array
					start_config.each_index do |i|
						j = 0
						start_config[i].each do |k, v|
							set_button(k, v, r: i, c: j)
							j += 1
						end
					end
				end
			end

			def [](i)
				@rows[i]
			end

			def set_button(name, command = nil, c: 0, r: 0)
				command ||= name

				@rows[r] ||= [];
				out_button = {text: name}
				if command =~ /^(http|tg)/
					out_button[:url] = command
				else
					out_button[:callback_data] = command
				end

				@rows[r][c] = out_button
			end

			def ilk_reply_markup()
				return { inline_keyboard: to_ilk_layout }
			end

			def to_ilk_layout()
				out_data = [];

				@rows.each do |row|
					next if row.nil?
					next if row.empty?

					formatted_row = []

					row.each do |button|
						next if button.nil?

						formatted_row << button
					end

					out_data << formatted_row unless formatted_row.empty?
				end

				out_data
			end
		end
	end
end
