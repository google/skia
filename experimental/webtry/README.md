WebTry
======

Allows trying out Skia code in the browser. Run a local webserver
and from the pages it serves try out Skia code and see the results
immediately. To make sandboxing easier this must be built w/GPU off.

Running Locally
===============

    $ GYP_GENERATORS=ninja  ./gyp_skia  gyp/webtry.gyp gyp/most.gyp -Dskia_gpu=0
    $ ninja -C out/Debug webtry
    $ cd experimental/webtry
    $ go build webtry.go
    $ ./webtry

Then visit http://localhost:8000 in your browser.

Only tested under linux, doubtful it will work on other platforms.

Full Server Setup
=================

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


Do once
-------

The following things only need to be done once

1. sudo apt-get install git schroot debootstrap
2. git clone https://skia.googlesource.com/skia
3. Add the following to /etc/fstab and reboot:

    none /dev/shm tmpfs rw,nosuid,nodev,noexec 0 0

The above will allow ninja to run. See http://stackoverflow.com/questions/2009278/python-multiprocessing-permission-denied

4. Add the following to the /etc/schroot/minimal/fstab:

    /home/webtry/inout             /inout  none    rw,bind         0       0

5. Change /etc/monit/monitrc to:

    set daemon 2

then run the following so it applies:

    sudo /etc/init.d/monit restart

This means that monit will poll every two seconds that our application is up and running.

6. Set the TCP keepalive. For more info see:
   https://developers.google.com/cloud-sql/docs/gce-access

    sudo bash -c 'echo 60 > /proc/sys/net/ipv4/tcp_keepalive_time'

Do the first time
-----------------

Do the following the first time you setup a machine, and each time you want to update the code running on the server

    cd ~/skia/experimental/webtry/setup
    ./webtry_setup.sh


Once, after setup
-----------------

Do this step only once, but only after running webtry_setup.sh the first time

    sudo debootstrap --variant=minbase wheezy /srv/chroot/webtry


Third Party Code
----------------

  * res/js/polyfill.js - Various JS polyfill libraries. To rebuild or update
    see polyfill/README.md.

