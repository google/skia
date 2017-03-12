#!/bin/bash
#
# android_install_app: installs the Skia development apps on the device.

function print_usage {
  echo "USAGE: android_install_app [options]"
  echo " Options:         -f  Forces the package to be installed by removing any"
  echo "                      previously installed packages"
  echo "                  -h  Prints this help message"
  echo "      -s [device_s/n] Serial number of the device to be used"
}

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

source $SCRIPT_DIR/utils/android_setup.sh
source $SCRIPT_DIR/utils/setup_adb.sh

forceRemoval="false"

for arg in ${APP_ARGS[@]}; do
  if [[ "${arg}" == "-f" ]]; then
    forceRemoval="true"
  elif [[ "${arg}" == "-h" ]]; then
    print_usage
    exit
  elif [[ ${arg} == '-'* ]]; then
    echo "ERROR: unrecognized option ${arg}"
    print_usage
    exit 1;
  fi
done

APP_LC=$(echo Viewer | tr "[:upper:]" "[:lower:]")

if [[ "$forceRemoval" == "true" ]];
then
    echo "Forcing removal of previously installed packages"
    $ADB ${DEVICE_SERIAL} uninstall org.skia.${APP_LC} > /dev/null
fi

echo "Installing ${APP_LC} from ${SKIA_OUT}/${APP_LC}.apk"
$ADB ${DEVICE_SERIAL} install -r ${SKIA_OUT}/${APP_LC}.apk

