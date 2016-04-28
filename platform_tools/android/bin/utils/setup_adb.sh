# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

UTIL_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

if [ $PYADB ] && [ -a "$PYADB" ]; then
  echo "Python ADB detected, going to use that"
  ADB="python ${PYADB}"
  return
fi

if [ "$(which adb)" != "" ]; then
    ADB="$(which adb)"
elif [ -d "$ANDROID_SDK_ROOT" ]; then
    ADB="${ANDROID_SDK_ROOT}/platform-tools/adb"
else
  echo $ANDROID_SDK_ROOT
  echo "No ANDROID_SDK_ROOT set (check that android_setup.sh was properly sourced)"
  exit 1
fi

if [ ! -x $ADB ]; then
  echo "The adb binary is not executable"
  exit 1
fi
