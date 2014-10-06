WebTry Server
=============

Allows trying out Skia code in the browser. Run a local webserver
and from the pages it serves try out Skia code and see the results
immediately. To make sandboxing easier this must be built w/GPU off.


Running Locally
===============

Set your SKIA_ROOT environment variable to point at your skia tree, then:

    $ cd experimental/webtry
    $ go get -d
    $ ./build
    $ ./webtry

Then visit http://localhost:8000 in your browser.

Only tested under linux and MacOS, doubtful it will work on other platforms.


Server Setup
============

Create a GCE instance:

    gcutil --project=google.com:skia-buildbots addinstance skia-webtry-b \
      --zone=us-central2-b --external_ip_address=108.170.220.126 \
      --service_account=default \
      --service_account_scopes="https://www.googleapis.com/auth/devstorage.full_control" \
      --network=default --machine_type=n1-standard-1 --image=backports-debian-7-wheezy-v20140331 \
      --persistent_boot_disk

Make sure port 80 is accessible externally for the above instance.

SSH into the instance:

    gcutil --project=google.com:skia-buildbots ssh --ssh_user=default skia-webtry-b


Do the first time
=================

The following things only need to be done once.

1. SSH into the server as default.

2. sudo apt-get install git schroot debootstrap

3. Add the following to the /etc/schroot/minimal/fstab:

  none /run/shm tmpfs rw,nosuid,nodev,noexec 0 0
  /home/webtry/inout             /skia_build/inout  none    rw,bind         0       0
  /home/webtry/cache             /skia_build/cache  none    rw,bind         0       0


4. git clone https://skia.googlesource.com/skia

5. cd ~/skia/experimental/webtry/setup

6. ./webtry_setup.sh

7. Change /etc/monit/monitrc to:

    set daemon 2

then run the following so it applies:

    sudo /etc/init.d/monit restart

This means that monit will poll every two seconds that our application is up and running.

8. Set the TCP keepalive. For more info see:
   https://developers.google.com/cloud-sql/docs/gce-access

    sudo bash -c 'echo 60 > /proc/sys/net/ipv4/tcp_keepalive_time'

To update the code
==================

1. SSH into the server as default.
2. cd ~/skia/experimental/webtry/setup
3. git pull
4. ./webtry_setup.sh

Third Party Code
================

  * res/js/polyfill.js - Various JS polyfill libraries. To rebuild or update
    see poly/README.md.
