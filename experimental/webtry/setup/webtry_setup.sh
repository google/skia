#!/bin/bash
#
# Script to setup a GCE instance to run the webtry server.
# For full instructions see the README file.
sudo apt-get install schroot debootstrap monit squid3
sudo apt-get install g++ libfreetype6 libfreetype6-dev libpng12-0 libpng12-dev libglu1-mesa-dev mesa-common-dev freeglut3-dev libgif-dev libfontconfig libfontconfig-dev

echo "Adding the webtry user account"
sudo adduser webtry

sudo cp continue_install /home/webtry/continue_install
sudo chmod 766 /home/webtry/continue_install
sudo chown webtry:webtry /home/webtry/continue_install
sudo su webtry -c /home/webtry/continue_install

sudo mkdir -p /srv/chroot/webtry
sudo cp /home/webtry/skia/experimental/webtry/sys/webtry_schroot /etc/schroot/chroot.d/webtry

sudo mkdir /srv/chroot/webtry/etc
sudo mkdir /srv/chroot/webtry/bin
sudo cp /bin/sh /srv/chroot/webtry/bin/sh

# Copy all the dependent libraries into the schroot.
sudo cp --parents `ldd /home/webtry/skia/out/Debug/webtry | cut -d " " -f 3` /srv/chroot/webtry
sudo cp --parents `ldd /bin/sh | cut -d " " -f 3` /srv/chroot/webtry

sudo cp /home/webtry/skia/experimental/webtry/sys/webtry_init /etc/init.d/webtry
sudo cp /home/webtry/skia/experimental/webtry/sys/webtry_monit /etc/monit/conf.d/webtry
sudo cp /home/webtry/skia/experimental/webtry/sys/webtry_squid /etc/squid3/squid.conf
sudo chmod 744 /etc/init.d/webtry

# Confirm that monit is happy.
sudo monit -t
