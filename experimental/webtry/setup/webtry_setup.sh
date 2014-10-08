#!/bin/bash
#
# Script to setup a GCE instance to run the webtry server.
# For full instructions see the README file.

function banner {
	echo ""
	echo "******************************************"
	echo "*"
	echo "* $1"
	echo "*"
	echo "******************************************"
	echo ""
}

banner "Installing debian packages needed for the server"

sudo apt-get install schroot debootstrap monit squid3

# although aufs is being replaced by overlayfs, it's not clear
# to me if overlayfs is completely supported by schroot yet.
sudo apt-get install aufs-tools

banner "Setting up the webtry user account"
sudo adduser webtry

sudo mkdir /home/webtry/cache
sudo mkdir /home/webtry/cache/src
sudo mkdir /home/webtry/inout
sudo chmod 777 /home/webtry/inout
sudo chmod 777 /home/webtry/cache
sudo chmod 777 /home/webtry/cache/src

sudo cp ../sys/webtry_schroot /etc/schroot/chroot.d/webtry

CHROOT_JAIL=/srv/chroot/webtry_gyp
# Build the chroot environment.
if [ ! -d ${CHROOT_JAIL} ]; then
	banner "Building the chroot jail"
	sudo mkdir -p ${CHROOT_JAIL}

	sudo debootstrap --variant=minbase wheezy ${CHROOT_JAIL}
	sudo cp setup_jail.sh ${CHROOT_JAIL}/bin
	sudo chmod 755 ${CHROOT_JAIL}/bin/setup_jail.sh
	sudo chroot ${CHROOT_JAIL} /bin/setup_jail.sh
	sudo sh -c "echo 'none /dev/shm tmpfs rw,nosuid,nodev,noexec 0 0' >> ${CHROOT_JAIL}/etc/fstab"
fi

# The continue_install_jail script will update and build up the skia library
# inside the jail.

banner "Installing and updating software on the chroot jail"
sudo cp continue_install_jail.sh ${CHROOT_JAIL}/bin/continue_install_jail.sh
sudo chmod 755 ${CHROOT_JAIL}/bin/continue_install_jail.sh
sudo chroot ${CHROOT_JAIL} /bin/continue_install_jail.sh
sudo chown -R webtry:webtry ${CHROOT_JAIL}/skia_build/skia

# The continue_install script will fetch the latest versions of
# skia and depot_tools.  We split up the installation process into
# two pieces like this so that the continue_install script can
# be run independently of this one to fetch and build the latest skia.

banner "Building the webtry server outside the jail"

sudo cp continue_install.sh /home/webtry
sudo chown webtry:webtry /home/webtry/continue_install.sh
sudo su - webtry -c /home/webtry/continue_install.sh

banner "Setting up system initialization scripts"

sudo cp ../sys/webtry_init /etc/init.d/webtry
sudo cp ../sys/webtry_monit /etc/monit/conf.d/webtry
sudo cp ../sys/webtry_squid /etc/squid3/squid.conf
sudo chmod 744 /etc/init.d/webtry

# Confirm that monit is happy.
sudo monit -t
sudo monit reload

banner "Restarting webtry server"

sudo /etc/init.d/webtry restart

banner "All done!"
