
require_relative 'Chat.rb'

module XNM
	module Telegram
		# Telegram User class.
		#
		# This class is an extension of a Telegram {Chat} object that
		# represents a DM with a User, and represents the user himself.
		# This is because Telegram's User ID is equivalent to the
		# Chat ID of the DM with the User.
		class User	< Chat
			# Username, without the @
			attr_reader :username
			# First name, always guaranteed to be set.
			attr_reader :first_name
			# Last name, may not be set.
			attr_reader :last_name

			# A human readable name.
			# Overrides {Chat#casual_name}
			attr_reader :casual_name

			# List of permissions this user has.
			# This Array of Strings will be used to check against
			# a executed command, to see if the User has appropriate rights
			# to run said command.
			#
			# Note that the {Handler#permissions_list} will be used to expand
			# this list, i.e. if the permissions list is:
			# `{ 'admin' => ['basic_rights'] }`
			# And the user has the 'admin' permission, he will also have
			# basic_rights *without them being listed in {#permissions}*
			#
			# Use {#has_permissions?} to check if a user has a certain permission.
			attr_reader :permissions

			# Permanent user state. Will be saved to disk.
			# @todo Actually save to disk.
			attr_reader :perm_state

			# Temporary state. Will be lost of restart, should be cleaned
			# out and only used for intermediary work.
			attr_accessor :temp_state

			# Initialize a new user object.
			#
			# Pass the handler used for this User as well as the
			# Hash containing Telegram's "User" Object.
			def initialize(handler, user_info)
				super(handler, user_info);

				@username   = user_info[:username]
				@first_name = user_info[:first_name]
				@last_name  = user_info[:last_name]

				@casual_name = user_info[:first_name]

				@permissions = user_info[:permissions] || []
				@perm_state  = user_info[:perm_state] || {}
			end

			def add_permissions(list)
				list = [list].flatten

				list.each do |perm|
					next if perm.is_a? Symbol
					next if perm.is_a? String

					raise ArgumentError, "Permission must be String or Symbol!"
				end

				@permissions = (@permissions + list).uniq
			end
			alias add_permission add_permissions

			def take_permissions(list)
				list = [list].flatten

				list.each do |perm|
					next if perm.is_a? Symbol
					next if perm.is_a? String

					raise ArgumentError, "Permission must be String or Symbol!"
				end

				@permissions -= list;
			end
			alias take_permission take_permissions

			# Check if a user has all given permissions.
			# Will run {#has_permission} against every permission
			# in the list, returns false if any one permission is not met.
			#
			# An empty list counts as true
			def has_permissions?(*targets)
				targets = targets.flatten

				return true if targets.nil?
				return true if @permissions.include? :sudo

				targets.each do |perm|
					return false unless has_permission? perm
				end

				true
			end

			# Check whether a user has a given permission.
			#
			# This function will check if the given permission is in the
			# User's {#permissions}. It will also use the
			# {Handler#permissions_list} to expand the user's permissions,
			# i.e. if the permissions list is:
			# `{ 'admin' => ['basic_rights'] }`
			# And the user has the 'admin' permission, he will also have
			# basic_rights *without them being listed in {#permissions}*
			#
			# nil will return true.
			#
			# @note This will always return true if the user has the :sudo
			#   permission, use only for full admin access!
			def has_permission?(target)
				return true if @permissions.include? :sudo
				return true if target.nil?

				unchecked = @permissions.dup
				checked = {}

				while perm = unchecked.pop
					next if checked[perm]
					return true if perm == target

					unchecked += @handler.permissions_list[perm] || []
					checked[perm] = true
				end

				false
			end
		end
	end
end
