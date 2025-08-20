Skia Infrastructure
===================

This directory contains infrastructure elements.


Tasks and Jobs
--------------

Files in this directory define a DAG of tasks which run at every Skia commit. A
task is a small, self-contained unit which runs via Swarming on a machine in the
pool. Tasks may be chained together, eg. one task to compile test binaries and
another to actually run them.

Jobs are collections of related tasks which help define sub-sections of the DAG,
for example, to be used as try jobs. Each job is defined as an entry point into
the DAG.

The tasks.json file in this directory is the list of tasks and jobs for
the repo. Note that tasks.json is NEVER edited by hand but generated via
gen_task.go and the input files enumerated below. The
[Task Scheduler](https://skia.googlesource.com/buildbot/+/main/task_scheduler/README.md)
reads the tasks.json file at each commit to determine which jobs to run. For
convenience, gen_tasks.go is provided to generate tasks.json and also to test it
for correct syntax and detecting cycles and orphaned tasks. Always edit
gen_tasks.go or one of the following input JSON files, rather than tasks.json
itself:

  * cfg.json - Basic configuration information for gen_tasks.go.
  * jobs.json - The list of all jobs to run. Edit this to add or remove
      bots.

Whenever gen_tasks.go, any of the above JSON files, or assets are changed, you
need to run gen_tasks.go to regenerate tasks.json:

	$ go run infra/bots/gen_tasks.go

Or:

	$ make -C infra/bots train

There is also a test mode which performs consistency checks and verifies that
tasks.json is unchanged:

	$ go run infra/bots/gen_tasks.go --test

Or:

	$ make -C infra/bots test


Upon adding a new job, you may want to test whether it runs successfully
before landing the change. However, the resultant Gerrit CL that gets created
from the code change that adds the new job will not automatically run it. It
also will not yet be available in Gerrit's UI in the list of available tryjobs.

In order to trigger it to run before landing the change, you can use the
SK CLI tool. See the [SK CLI Tool docs](https://chromium.googlesource.com/skia/+/HEAD/site/docs/dev/tools/sk.md) for more information.

If you haven't yet fetched it from your skia directory, you can do so by
running the following command.

	$ ./bin/fetch-sk

To kick off jobs, you must be logged in to luci-auth.

	$ luci-auth login

You can then use `sk try` to initiate jobs with this change:

	$ ./bin/sk try [name of job]

For example, if you wanted to run
"Test-Mac14-Clang-MacMini9.1-GPU-AppleM1-arm64-Debug-All", that command would be

	$ ./bin/sk try Test-Mac14-Clang-MacMini9.1-GPU-AppleM1-arm64-Debug-All

This change's Gerrit review page UI should now display the job's status.

Recipes
-------

Recipes are the framework used by Skia's infrastructure to perform work inside
of Swarming tasks. The main elements are:

  * recipes.py - Used for running and testing recipes.
  * recipes - These are the entry points for each type of task, eg. compiling
      or running tests.
  * recipe_modules - Shared modules which are used by recipes.
  * .recipe_deps - Recipes and modules may depend on modules from other repos.
      The recipes.py script automatically syncs those dependencies in this
      directory.


Isolate Files
-------------

These files determine which parts of the repository are transferred to the bot
when a Swarming task is triggered. The
[Isolate tool](https://github.com/luci/luci-py/tree/main/appengine/isolate/doc)
hashes each file and will upload any new/changed files. Bots maintain a cache so
that they can efficiently download only the files they don't have.


Assets
------

Artifacts used by the infrastructure are versioned here, along with scripts for
recreating/uploading/downloading them. See the README in that directory for more
information. Any time an asset used by the bots changes, you need to re-run
gen_tasks.go.


Tools
-----

Assorted other infrastructure-related tools, eg. isolate and CIPD binaries.


CT
--

Helpers for running Skia tasks in Cluster Telemetry.

