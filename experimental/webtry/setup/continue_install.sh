#!/bin/bash
#
# The continue_install script updates the jail's copy of depot tools and the latest
# verion of skia.  It then builds the skia libraries inside the jail, and builds the webtry
# server outside the jail.
#
# See the README file for detailed installation instructions.

CHROOT_JAIL=/srv/chroot/webtry_gyp
sudo cp continue_install_jail.sh ${CHROOT_JAIL}/bin/continue_install_jail.sh
sudo chmod 755 ${CHROOT_JAIL}/bin/continue_install_jail.sh
sudo chroot ${CHROOT_JAIL} /bin/continue_install_jail.sh
sudo chown -R webtry:webtry ${CHROOT_JAIL}/skia_build/skia
cd

# Install Go
if [ -d go ]; then
  echo Go already installed.
else
  wget https://storage.googleapis.com/golang/go1.3.3.linux-amd64.tar.gz
  tar -xzf go1.3.3.linux-amd64.tar.gz
fi

mkdir ${HOME}/golib
export GOROOT=${HOME}/go
export GOPATH=${HOME}/golib
export PATH=$PATH:$GOROOT/bin

cd skia/experimental/webtry

go get -d
./build
