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


<a name="connecting-to-swarming-bots"></a>
Connecting to Swarming Bots
---------------------------

If you need to make changes on a bot/device, please check with the Trooper or another Infra team member. Most
bots/devices can be flashed/imaged back to a clean state, but others can not.

- Machine name like “skia-e-gce-NNN”, “skia-ct-gce-NNN”, “skia-i-gce-NNN”, “ct-gce-NNN”, “ct-xxx-builder-NNN” -> GCE
  * First determine the project for the bot:
     + skia-e-gce-NNN, skia-ct-gce-NNN: [skia-swarming-bots](https://console.cloud.google.com/compute/instances?project=skia-swarming-bots)
     + skia-i-gce-NNN: [google.com:skia-buildbots](https://console.cloud.google.com/compute/instances?project=google.com:skia-buildbots)
     + ct-gce-NNN, ct-xxx-builder-NNN: [ct-swarming-bots](https://console.cloud.google.com/compute/instances?project=ct-swarming-bots)
  * To log in to a Linux bot in GCE, use `gcloud compute ssh --project <project> default@<machine name>`. Choose the zone listed on the VM's detail page (see links above). You may also specify the zone using the `--zone` command-line flag.
  * To log in to a Windows bot in GCE, first go to the VM's detail page and click the "Set Windows password"
    button. (Alternatively, ask the Infra Team how to log in as chrome-bot.) There are two options to connect:
     + SSH: Follow the instructions for Linux using your username rather than `default`.
     + RDP: On the VM's detail page, click the "RDP" button. (You will be instructed to install the Chrome RDP Extension
       for GCP if it hasn't already been installed.)

- Machine name ends with “a9”, “m3”, "m5" -> Chrome Golo/Labs
  * To log in to Golo bots, see [go/chrome-infra-build-access](https://goto.google.com/chrome-infra-build-access).

- Machine name starts with “skia-e-”, “skia-i-” (other than “skia-i-gce-NNN”), “skia-rpi-” -> Chapel Hill lab (aka Skolo)<br/>
  To log in to Skolo bots, see the [Skolo maintenance doc][remote access] remote access section. See the following for OS specific instructions:<br/>
  * [Remotely debug an Android device in Skolo][remotely debug android]
  * [VNC to Skolo Windows bots][vnc to skolo windows]
  * [ChromeOS Debugging][chromeos debugging]


Debugging
---------

If you need to run code on a specific machine/device to debug an issue, the simplest option is to
run tryjobs (after adding debugging output to the relevant code). In some cases you may also need to
[create or modify tryjobs](automated_testing#adding-new-jobs).

For Googlers: If you need more control (e.g. to run GDB) and need to run directly on a swarming bot then you can use [leasing.skia.org](https://leasing.skia.org).<br/>
If that does not work then the [current trooper][current trooper] can help you bring the device back to your desk and connect
it to GoogleGuest Wifi or the [Google Test Network](http://go/gtn-criteria).

If you need to make changes on a bot/device, please check with the Trooper or another Infra team member. Most
bots/devices can be flashed/imaged back to a clean state, but others can not.

If a permanent change needs to be made on the machine (such as an OS or driver update), please [file
a bug][infra bug] and assign to jcgregorio for reassignment.

For your convenience, the machine skolo-builder is available for checking out and compiling code within the Skolo. See
more info in the [Skolo maintenance doc][remote access] remote access section.

[current trooper]: http://tree-status.skia.org/trooper
[remote access]:
    https://docs.google.com/document/d/1zTR1YtrIFBo-fRWgbUgvJNVJ-s_4_sNjTrHIoX2vulo/edit#heading=h.v77cmwbwc5la
[infra bug]: https://bugs.chromium.org/p/skia/issues/entry?template=Infrastructure+Bug
[remotely debug android]: https://docs.google.com/document/d/1nxn7TobfaLNNfhSTiwstOnjV0jCxYUI1uwW0T_V7BYg/
[vnc to skolo windows]:
    https://docs.google.com/document/d/1zTR1YtrIFBo-fRWgbUgvJNVJ-s_4_sNjTrHIoX2vulo/edit#heading=h.7cqd856ft0s
[chromeos debugging]:
    https://docs.google.com/document/d/1yJ2LLfLzV6pXKjiameid1LHEz1mj71Ob4wySIYxlBdw/edit#heading=h.9arg79l59xrf

Maintenance Tasks
-----------------

See the [Skolo maintenance doc][skolo maintenance].

[skolo maintenance]:
    https://docs.google.com/document/d/1zTR1YtrIFBo-fRWgbUgvJNVJ-s_4_sNjTrHIoX2vulo/edit
