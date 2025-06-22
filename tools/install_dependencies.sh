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

rpm_all_installed() {
    for arg; do
        if ! rpm -q "$arg" >/dev/null 2>&1; then
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
		libgl1-mesa-dev
		libglu1-mesa-dev
		libharfbuzz-dev
		libicu-dev
		libjpeg-dev
		libpng-dev
		libwebp-dev
		libx11-xcb-dev
		libxcb-xkb-dev
		xcb
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
        Fedora|CentOS|RedHatEnterprise*|Rocky*|AlmaLinux*)
            PACKAGES=$(cat<<-EOF
		gcc-c++
		freeglut-devel
		fontconfig-devel
		freetype-devel
		mesa-libGL-devel
		mesa-libGLU-devel
		harfbuzz-devel
		libicu-devel
		libjpeg-turbo-devel
		libpng-devel
		libwebp-devel
		libxcb-devel
		xcb-util-keysyms-devel
		ninja-build
		EOF
        )
            if ! rpm_all_installed $PACKAGES; then
                if command -v dnf > /dev/null 2>&1; then
                    sudo dnf $1 install $PACKAGES
                else
                    sudo yum $1 install $PACKAGES
                fi
            fi
            exit
            ;;
    esac
fi

if [ -f /etc/redhat-release ]; then
    PACKAGES=$(cat<<-EOF
	gcc-c++
	freeglut-devel
	fontconfig-devel
	freetype-devel
	mesa-libGL-devel
	mesa-libGLU-devel
	harfbuzz-devel
	libicu-devel
	libjpeg-turbo-devel
	libpng-devel
	libwebp-devel
	libxcb-devel
	xcb-util-keysyms-devel
	ninja-build
	EOF
    )
        if ! rpm_all_installed $PACKAGES; then
            if command -v dnf > /dev/null 2>&1; then
                sudo dnf install $PACKAGES
            else
                sudo yum install $PACKAGES
            fi
        fi
    exit
fi

echo 'unknown system'
exit 1
