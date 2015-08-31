SkiaLab
=======

Overview
--------

Skia's buildbots are hosted in three places:

* Google Compute Engine. This is the preferred location for bots which don't
  need to run on physical hardware, ie. anything that doesn't require a GPU,
  stable performance numbers, or a specific hardware configuration. Most of our
  compile bots live here, along with some non-GPU test bots on Linux and
  Windows.
* Chrome Golo. This is the preferred location for bots which require specific
  hardware or OS configurations that are not supported by GCE. We have several
  Mac, Linux, and Windows bots in the Golo.
* The local SkiaLab in Chapel Hill. Anything we can't get in GCE or the Golo
  lives here. This includes newer or uncommon GPUs and all Android, ChromeOS,
  and iOS devices.

This page covers the local SkiaLab in Chapel Hill.


Layout
------

The SkiaLab consists of three wireframe racks which hold machines connected to
two KVM switches. Each KVM switch has a monitor, mouse, and keyboard and is the
primary mode of access to the lab machines. In general, the machines are on the
same rack as the KVM switch used to access them. The switch nearest the door
(labeled "DOOR"), is connected to machines on its own rack as well as a smaller
rack closer to the door.

Each machine is labeled with its hostname and the number or letter used to
access it on the KVM switch. Android devices are located on the rack nearest
the interior of the office (the KVM switch is labeled "OFFICE"). They are
labeled with their serial number and the name of the buildslave they are
associated with. Each device connects to a host machine, either directly or
by way of a powered USB hub.

**Disclaimer: Please ONLY make changes on a lab machine as a last resort, as it
is disruptive to the running bots and can leave the machines in a dirty state.
If you must make changes, such as cloning a copy of Skia to run tests and debug
failures, be sure to clean up after yourself. If a permanent change needs to be
made on the machine (such as a driver update), please contact an infra team
member.**


Common Tasks
------------

### Locating the host machine for a failing bot

Sometimes failures can only be reproduced on a particular hardware
configuration. In these cases, it is sometimes necessary to log into the host
machine where a failing bot is running in order to debug the failure.

From the [Status](https://status.skia.org/) page:

1. Click on the box associated with a failed build.
2. A popup will appear with some information about the build, including the
   builder and buildslave. Click the "Lookup" link next to "Host machine". This
   will bring you to the [SkiaLab Hosts](https://status.skia.org/hosts) page,
   which contains information about the machines in the lab, pre-filtered to
   select the machine which runs the buildslave in question.
3. The information box will display the hostname of the machine as well as the
   KVM switch and number used to access the machine, if the machine is in the
   SkiaLab.
4. Walk over to the lab. While standing at the KVM switch indicated by the host
   information page, double tap \<ctrl\> and then press the number or letter from
   the information page. It may be necessary to move or click the mouse to wake
   the machine up.
5. Log in to the machine if necessary. The password is stored in
   [Valentine](https://valentine/) as "Chapel Hill buildbot slave password".

### Rebooting a problematic Android device

Follow the same process as above, with some slight changes:

1. On the [Status](https://status.skia.org/) page, click the box for the failed
   build.
2. Click the "Lookup" link for the host machine. Remember the name of the
   buildslave which ran the build.
3. The hosts page will display the information used to access the host machine
   for the device as well as the serial number for the device next to the name
   of its buildsave.
4. Walk over to the lab and find the Android device with the serial number from
   the hosts page. Hold the power and volume-up buttons until the device
   reboots.
5. Access the host machine for the device, per the above instructions. Use the
   `which_devices.py` script to verify that the device has re-attached. From
   the home directory:

        $ python buildbot/scripts/which_devices.py


Maintenance Tasks
-----------------

### Bringing up a new buildbot host machine

This assumes that we're just adding a host machine for a new buildbot slave,
and doesn't cover how to make changes to the buildbot code to change the
behavior of the builder itself.

1. Obtain the machine itself and place it on the racks in the lab. Connect
   power, ethernet, and KVM cables.
2. If we already have a disk image appropriate for this machine, follow the
   instructions for flashing a disk image to a machine below. Otherwise, follow
   the instructions for bringing up a new machine from scratch.
3. Power on the machine. Be sure to kill any buildbot processes that start up,
   eg. `killall python` on Linux and Mac, and just close any cmd instances which
   pop up on Windows.
4. Set the hostname for the machine.
5. Ensure that the machine is labeled with its hostname and KVM number.
6. Add the new slave to the slaves.cfg file on the appropriate master, eg.
   https://chromium.googlesource.com/chromium/tools/build/+/master/masters/master.client.skia/slaves.cfg,
   and upload the change for code review.
7. Add an entry for the new host machine to the slave_hosts_cfg.py file in the
   Skia infra repo: https://skia.googlesource.com/buildbot/+/master/site_config/slave_hosts_cfg.py,
   and upload it for review.
8. Commit the change to add the slave to the master. Once it lands, commit the
   slave_hosts_cfg.py change immediately afterward.
9. Restart the build master. Either ask borenet@ to do this or file a
   [ticket](https://code.google.com/p/chromium/issues/entry?template=Build%20Infrastructure&labels=Infra-Labs,Restrict-View-Google,Infra-Troopers&summary=Restart%20request%20for%20[%20name%20]&comment=Please%20provide%20the%20reason%20for%20restart.%0A%0ASet%20to%20Pri-0%20if%20immediate%20restarted%20is%20required,%20otherwise%20please%20set%20to%20Pri-1%20and%20the%20restart%20will%20happen%20when%20the%20trooper%20gets%20a%20free%20moment.) for a trooper to do it.
10. Reboot the machine and monitor the build master to ensure that it connects.
    This can take some time, since the bot needs to sync Chrome.


### Bringing up a new Android bot

1. Locate or add a host machine. We generally want to keep the number of
   devices attached to each host below 5 or so. If a new host machine is
   required, follow the above instructions for bringing up a new buildbot
   host machine, with the exception that the slave corresponds to the Android
   device, not the host machine itself.
2. Ensure that the buildslave is not yet running:

        $ killall python

3. Disable MTP and PTP on the device.  Some devices require one or the other to
   be enabled; in that case, select PTP and choose to 'do nothing' when
   attaching to the host machine.
4. Connect the device to the host machine, either through a powered USB hub or
   directly to the machine.
5. Make sure that the device is in developer mode and that USB debugging is
   enabled.
6. Authorize the device for USB debugging on the host machine by checking the
   "always allow" box on dialog box which appears on the Android device after
   plugging it into the host.
7. Ensure that the device appears as "connected" when you run the
   `which_devices.py` script:

        $ python buildbot/scripts/which_devices.py

8. Reboot the machine to start the buildslave.


### Bringing up a new machine from scratch

TODO(borenet): Migrate from Google Docs.

OS-specific instructions are available in a
[Google Doc](https://docs.google.com/document/d/1X7Hvsj33AlBmj-KEWfFbmdCArUJJAICLkB7ipDcxRV8/edit)


### Flashing a disk image to a machine

1. Find the USB key labeled, "Clonezilla" in the SkiaLab and insert it into the
   machine.
2. Turn on the machine and load the boot menu. For Shuttle machines, press
   \<del\> or \<esc\>. Mac machines require that you plug in the Mac keyboard and
   press the \<option\> key at boot. Boot from the USB key. It's typically UEFI
   and named something like "FlashBlu" or "Kanguru".
3. At the Clonezilla menu, choose the "to RAM" option.
4. Choose your preferred language.
5. "Don't touch keymap".
6. "Start Clonezilla".
7. "device-image".
8. "local_dev".
9. Unplug the flash drive and plug in the external hard drive labeled, "Disk
   images." Wait for the "Attached Enclosure device" message to appear, then
   hit \<enter\>.
10. Select the external drive to use for /home/partimag, something like,
    "1000GB_ntfs_My_Passport".
11. Select the bot_img directory.
12. Hit \<enter\> to continue.
13. "Beginner"
14. "restoredisk"
15. Select the image to use. Make sure that it's compatible with this machine.
16. Choose the hard drive in the machine. It should be the only option.
17. "y" and "y"
18. Choose "reboot" after flashing the image to the machine.
19. Set the hostname of the machine so that it doesn't conflict with any
    existing machines.

### Capturing a disk image

1. Make sure that the machine is in a clean state: no pre-existing buildslave
   checkouts, extra software, etc.
2. Find the USB key labeled, "Clonezilla" in the SkiaLab and insert it into the
   machine.
3. Turn on the machine and load the boot menu. For Shuttle machines, press
   \<del\> or \<esc\>. Mac machines require that you plug in the Mac keyboard and
   press the \<option\> key at boot. Boot from the USB key. It's typically UEFI
   and named something like "FlashBlu" or "Kanguru".
4. At the Clonezilla menu, choose the "to RAM" option.
5. Choose your preferred language.
6. "Don't touch keymap".
7. "Start Clonezilla".
8. "device-image".
9. "local_dev"
10. Unplug the flash drive and plug in the external hard drive labeled, "Disk
    images." Wait for the "Attached Enclosure device" message to appear, then
    hit \<enter\>.
11. Select the external drive to use for /home/partimag, something like,
    "1000GB_ntfs_My_Passport".
12. Select the bot_img directory.
13. "Beginner"
14. "savedisk"
15. Choose a name for the disk image. The convention is:
    `skiabot-<hardware type>-<OS>-<disk image revision #>`
12. Choose the hard drive in the machine. It should be the only option.
13. "y"
14. Choose "reboot" or "shut down" when finished.
