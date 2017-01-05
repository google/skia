#!/bin/bash
#
# android_launch_app: Launches the skia Viewer app on the device.

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source $SCRIPT_DIR/utils/android_setup.sh
source $SCRIPT_DIR/utils/setup_adb.sh

# TODO: check to ensure that the app exists on the device and prompt to install

if [[ -n $RESOURCE_PATH ]]; then
  adb_push_if_needed "${SCRIPT_DIR}/../../../resources" $RESOURCE_PATH
fi

activity="org.skia.viewer/org.skia.viewer.ViewerActivity"
$ADB ${DEVICE_SERIAL} shell "am start -S -n ${activity} --es cmdLineFlags \"${APP_ARGS[*]:1}\""
