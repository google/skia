#!/bin/sh
# Copyright 2014 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# install_dependencies.sh will install system-specific Skia
# dependencies using your system's package manager.  If your system is
# not supported, add logic here to support it.

set -e

if command -v lsb_release > /dev/null ; then
    case $(lsb_release -i -s) in
        Ubuntu)
            sudo apt-get install \
                build-essential \
		libfreetype6-dev \
		libfontconfig-dev \
		libpng12-dev \
		libgif-dev \
		libqt4-dev \
		clang
	    if [ $(lsb_release -r -s) = '14.04' ] ; then
		sudo apt-get install \
		    ninja-build
	    fi
            exit
            ;;
    esac
fi

echo 'unknown system'
exit 1

