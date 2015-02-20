#!/bin/bash
#

UTIL_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

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

if [ $(uname) == "Linux" ]; then
  ADB_REQUIRED="1.0.32"
elif [ $(uname) == "Darwin" ]; then
  ADB_REQUIRED="1.0.31 or 1.0.32"
fi

# get the version and then truncate it to be just the version numbers
ADB_VERSION="$($ADB version)"
ADB_VERSION="${ADB_VERSION##* }"

if [[ "$ADB_REQUIRED" != *"$ADB_VERSION"* ]]; then
  echo "WARNING: Your ADB version is out of date!"
  echo "  Expected ADB Version: ${ADB_REQUIRED}"
  echo "  Actual ADB Version: ${ADB_VERSION}"
fi
