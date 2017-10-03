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


Debugging
---------

If you need to run code on a specific machine/device to debug an issue, the simplest option is to
run tryjobs (after adding debugging output to the relevant code). In some cases you may also need to
[create or modify tryjobs](automated_testing#adding-new-jobs).

For Googlers: If you need more control (e.g. to run GDB), the [current Trooper][current trooper] can
loan a machine/device from the Skolo. All bots are accessible via either SSH or VNC -- see the
[Skolo maintenance doc remote access section][remote access] and/or get help from the Trooper. You
can also bring the device back to your desk and connect it to GoogleGuest WiFi or the [Google Test
Network](http://go/gtn-criteria).

If you need to make changes on a Skolo device, please check with an Infra team member. Most can be
flashed/imaged back to a clean state, but others can not.

If a permanent change needs to be made on the machine (such as an OS or driver update), please [file
a bug][infra bug] and assign to jcgregorio for reassignment.

[current trooper]: http://skia-tree-status.appspot.com/trooper
[remote access]:
    https://docs.google.com/document/d/1zTR1YtrIFBo-fRWgbUgvJNVJ-s_4_sNjTrHIoX2vulo/edit#heading=h.2nq3yd1axg0n
[infra bug]: https://bugs.chromium.org/p/skia/issues/entry?template=Infrastructure+Bug

Maintenance Tasks
-----------------

See the [Skolo maintenance doc][skolo maintenance].

[skolo maintenance]:
    https://docs.google.com/document/d/1zTR1YtrIFBo-fRWgbUgvJNVJ-s_4_sNjTrHIoX2vulo/edit
