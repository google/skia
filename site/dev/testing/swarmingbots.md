Skia Swarming Bots
==================

Overview
--------

Skia's Swarming bots are hosted in three places:

* Google Compute Engine. This is the preferred location for bots which don't need to run on physical
  hardware, ie. anything that doesn't require a GPU or a specific hardware configuration. Most of
  our compile bots live here, along with some non-GPU test bots on Linux and Windows. We get
  surprisingly stable performance numbers from GCE, despite very few guarantees about the physical
  hardware.
* Chrome Golo. This is the preferred location for bots which require specific hardware or OS
  configurations that are not supported by GCE. We have several Mac, Linux, and Windows bots in the
  Golo.
* The Skolo (local Skia lab in Chapel Hill). Anything we can't get in GCE or the Golo lives
  here. This includes a wider variety of GPUs and all Android, ChromeOS, iOS, and other devices.

[go/skbl](https://goto.google.com/skbl) lists all Skia Swarming bots.

Adding new jobs
---------------

See [Skia Automated Testing](automated_testing) for an overview of how jobs and tasks are executed
by the Skia Task Scheduler.

If you would like to add jobs to build or test new configurations, please file a [New Bot
Request](https://bugs.chromium.org/p/skia/issues/entry?template=New+Bot+Request).

If you know that the new jobs will need new hardware or you aren't sure which existing bots should
run the new jobs, assign to jcgregorio. Once the Infra team has allocated the hardware, we will
assign back to you to complete the process.

Generally it's possible to copy an existing job and make changes to accomplish what you want. You
will need to add the new job to
[infra/bots/jobs.json](https://skia.googlesource.com/skia/+/master/infra/bots/jobs.json). In some
cases, you will need to make changes to recipes:

* If there are new GN flags or compiler options:
  [infra/bots/recipe_modules/flavor/gn_flavor.py](https://skia.googlesource.com/skia/+/master/infra/bots/recipe_modules/flavor/gn_flavor.py)
* If there are modifications to dm flags:
  [infra/bots/recipes/test.py](https://skia.googlesource.com/skia/+/master/infra/bots/recipes/test.py)
* If there are modifications to nanobench flags:
  [infra/bots/recipes/perf.py](https://skia.googlesource.com/skia/+/master/infra/bots/recipes/perf.py)

If you need to do something more complicated, or if you are not sure how to add and configure the
new jobs, please ask for help from borenet, benjaminwagner, or mtklein.

Debugging
---------

If you need a physical machine/device to debug an issue, the [current
Trooper](http://skia-tree-status.appspot.com/trooper) can loan one from the Skolo. For Internet
access, you can connect to GoogleGuest WiFi.

If you need to make changes on a Skolo device, please check with an Infra team member. Most can be
flashed/imaged back to a clean state, but others can not.

If a permanent change needs to be made on the machine (such as an OS or driver update), please [file
a bug](https://bugs.chromium.org/p/skia/issues/entry?template=Infrastructure+Bug) and assign to
jcgregorio for reassignment.


Maintenance Tasks
-----------------

See the [Skolo maintenance
doc](https://docs.google.com/document/d/1zTR1YtrIFBo-fRWgbUgvJNVJ-s_4_sNjTrHIoX2vulo/edit).
