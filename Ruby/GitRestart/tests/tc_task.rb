
require_relative "../lib/git-restart/task.rb"

class Test_Task < Minitest::Test
	def setup()
		`rm /tmp/TEST_FILE_* 2>/dev/null`
	end

	def teardown()
		`rm /tmp/TEST_FILE_* 2>/dev/null`
	end

	def test_init
		# Test if the whole code starts normally
		@task = GitRestart::Task.new() do |t|
			t.name = "Test-Task"
		end
		@task.valid?

		# Check if signals are checked properly
		@task.signal = nil;
		@task.valid?

		@task.signal = "NonexistantSignal"
		assert_raises { @task.valid? }
		@task.signal = "INT"
	end

	def test_abort_on_error
		@task = GitRestart::Task.new() do |t|
			t.name = "TestTask"

			t.targets << "touch /tmp/TEST_FILE_1";
			t.targets << "exit 1";
			t.targets << "touch /tmp/TEST_FILE_2";
		end

		@task.start();
		@task.join();

		assert_equal 1, @task.lastStatus;
		assert File.exist?("/tmp/TEST_FILE_1");
		refute File.exist?("/tmp/TEST_FILE_2");
	end

	def test_stop_execution
		@task = GitRestart::Task.new() do |t|
			t.name = "TestTask"

			t.targets << "touch /tmp/TEST_FILE_1";
			t.targets << "sleep 10";
			t.targets << "touch /tmp/TEST_FILE_2";
		end

		@task.start();
		sleep(0.5);
		@task.stop();
		@task.join();

		assert File.exist?("/tmp/TEST_FILE_1");
		refute File.exist?("/tmp/TEST_FILE_2");
	end

	def test_complete_execution
		@task = GitRestart::Task.new() do |t|
			t.name = "TestTask"

			t.targets << "touch /tmp/TEST_FILE_1";
			t.targets << "sleep 1";
			t.targets << "touch /tmp/TEST_FILE_2";

			t.signal = nil;
		end

		@task.start();
		sleep 0.5;
		@task.stop();
		@task.join();

		assert File.exist?("/tmp/TEST_FILE_1");
		assert File.exist?("/tmp/TEST_FILE_2");
	end

	def test_chdir
		@task = GitRestart::Task.new() do |t|
			t.name = "TestTask";
			t.chdir = "/tmp";

			t.targets << "touch TEST_FILE_1";
		end

		@task.start();
		@task.join();

		assert File.exist?("/tmp/TEST_FILE_1");
	end

	def test_triggers()
		GitRestart::Task.branch 	="master";
		GitRestart::Task.modified 	= ["Tests/Test1/Test.rb"];

		@task = GitRestart::Task.new() do |t|
			t.chdir = "Tests/Test1";
			t.name = "TestTask"

			t.watch(%r{.*\.rb});
			assert t.triggered;
			t.triggered = false;

			t.watch("Test.rb");
			assert t.triggered;
			t.triggered = false;

			t.watch("NotTest.rb");
			refute t.triggered;

			t.on_branches ["master", "dev"];
			assert t.active;
			t.active = false;

			t.on_branches "dev";
			refute t.active;
		end
	end
end
