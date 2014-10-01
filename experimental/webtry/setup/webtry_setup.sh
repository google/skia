#!/bin/bash
#
# Script to setup a GCE instance to run the webtry server.
# For full instructions see the README file.
sudo apt-get install schroot debootstrap monit squid3

# although aufs is being replaced by overlayfs, it's not clear
# to me if overlayfs is completely supported by schroot yet.
sudo apt-get install aufs-tools

echo "Adding the webtry user account"
sudo adduser webtry

sudo mkdir /home/webtry/cache
sudo mkdir /home/webtry/cache/src
sudo mkdir /home/webtry/inout
sudo chmod 777 /home/webtry/inout
sudo chmod 777 /home/webtry/cache
sudo chmod 777 /home/webtry/cache/src

sudo cp ../sys/webtry_schroot /etc/schroot/chroot.d/webtry

CHROOT_JAIL=/srv/chroot/webtry_gyp
# Build the chroot environment
if [ ! -d ${CHROOT_JAIL} ]; then
	sudo mkdir -p ${CHROOT_JAIL}

	sudo debootstrap --variant=minbase wheezy ${CHROOT_JAIL}
	sudo cp setup_jail.sh ${CHROOT_JAIL}/bin
	sudo chmod 755 ${CHROOT_JAIL}/bin/setup_jail.sh
	sudo chroot ${CHROOT_JAIL} /bin/setup_jail.sh
	sudo echo "none /dev/shm tmpfs rw,nosuid,nodev,noexec 0 0" >> ${CHROOT_JAIL}/etc/fstab
fi

# the continue_install script will fetch the latest versions of 
# skia and depot_tools.  We split up the installation process into 
# two pieces like this so that the continue_install script can
# be run independently of this one to fetch and build the latest skia.

./continue_install.sh

sudo cp ../sys/webtry_init /etc/init.d/webtry
sudo cp ../sys/webtry_monit /etc/monit/conf.d/webtry
sudo cp ../sys/webtry_squid /etc/squid3/squid.conf
sudo chmod 744 /etc/init.d/webtry

# Confirm that monit is happy.

sudo monit -t
sudo monit reload
