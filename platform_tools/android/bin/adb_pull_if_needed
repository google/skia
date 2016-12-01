#!/bin/bash
#
# Copy the contents of a directory from a device to the host.

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source $SCRIPT_DIR/utils/android_setup.sh
source $SCRIPT_DIR/utils/setup_adb.sh

adb_pull_if_needed ${APP_ARGS[@]}
exit $?
