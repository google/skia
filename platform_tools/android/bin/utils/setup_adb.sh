#!/bin/bash
#

UTIL_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

if [ "$(which adb)" != "" ]; then
    ADB="$(which adb)"
elif [ $(uname) == "Linux" ]; then
    ADB=$UTIL_DIR/../linux/adb
elif [ $(uname) == "Darwin" ]; then
    ADB=$UTIL_DIR/../mac/adb
else
    echo "ERROR: Could not find ADB!"
    exit 1;
fi

#echo "ADB is: $ADB"
