#!/bin/bash
#
# Copy the contents of a directory from the host to a device.

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source $SCRIPT_DIR/utils/android_setup.sh
source $SCRIPT_DIR/utils/setup_adb.sh

adb_push_if_needed ${APP_ARGS[@]}
exit $?
