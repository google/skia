#!/bin/bash
#
# Author: Ravi Mistry
#
# Script to checkout and build a fresh copy of Chromium from head that uses a
# writable, tip-of-tree Skia rather than the read-only, revision-locked Skia
# specified in http://src.chromium.org/viewvc/chrome/trunk/src/DEPS
#
# Sample Usage:
#   tools/build-tot-chromium.sh ~/chromiumtrunk

if [[ $# -ne 1 ]] ; then
  echo "usage: $0 chromium_location"
  exit 1
fi
CHROMIUM_LOCATION=$1

echo -e "\n\n========Deleting $CHROMIUM_LOCATION========\n\n"
rm -rf $CHROMIUM_LOCATION

mkdir $CHROMIUM_LOCATION
cd $CHROMIUM_LOCATION
gclient config https://src.chromium.org/chrome/trunk/src
echo '
solutions = [
  { "name"        : "src",
    "url"         : "https://src.chromium.org/chrome/trunk/src",
    "deps_file"   : "DEPS",
    "managed"     : True,
    "custom_deps" : {
      "src/third_party/skia": "https://skia.googlecode.com/svn/trunk",
      "src/third_party/skia/gyp": None,
      "src/third_party/skia/src": None,
      "src/third_party/skia/include": None,
    },
    "safesync_url": "http://chromium-status.appspot.com/lkgr",
  },
]
' > .gclient

echo -e "\n\n========Starting gclient sync========\n\n"
START_TIME=$SECONDS
gclient sync
END_TIME=$SECONDS
echo -e "\n\n========gclient sync took $((END_TIME - START_TIME)) seconds========\n\n"

cd src
rm -rf out/Debug out/Release
GYP_GENERATORS='ninja' ./build/gyp_chromium

echo -e "\n\n========Starting ninja build========\n\n"
START_TIME=$SECONDS
ninja -C out/Release chrome
END_TIME=$SECONDS
echo -e "\n\n========ninja build took $((END_TIME - START_TIME)) seconds========\n\n"

SVN_VERSION=`svnversion .`
echo -e "\n\n========The Chromium & Skia versions are $SVN_VERSION========\n\n"

