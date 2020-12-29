#!/bin/sh
# Copyright 2014 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# install_dependencies.sh will install system-specific Skia
# dependencies using your system's package manager.  If your system is
# not supported, add logic here to support it.

# Pass in --yes as the first argument to force apt-get to skip Y/n prompts while
# being backward compatible with the old behavior.

set -e

# Return 0 iff all package name arguments are installed.
dpkg_all_installed() {
    for arg; do
        if !(dpkg-query -W -f'${Status}' "$arg" 2>/dev/null | \
            grep -q "ok installed"); then
            return 1
        fi
    done
    return 0
}

if command -v lsb_release > /dev/null ; then
    case $(lsb_release -i -s) in
        Ubuntu|Debian)
            PACKAGES=$(cat<<-EOF
		build-essential
		freeglut3-dev
		libfontconfig-dev
		libfreetype6-dev
		libgif-dev
		libgl1-mesa-dev
		libglu1-mesa-dev
		libharfbuzz-dev
		libicu-dev
		libjpeg-dev
		libpng-dev
		libwebp-dev
		EOF
            )
           if [ $(lsb_release -r -s) = '14.04' ] ; then
               PACKAGES="${PACKAGES} ninja-build"
           fi
           if ! dpkg_all_installed $PACKAGES; then
               sudo apt-get $1 install $PACKAGES
           fi
           exit
           ;;
    esac
fi

echo 'unknown system'
exit 1
