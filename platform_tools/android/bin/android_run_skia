#!/bin/bash
#
# android_run_skia: starts the correct skia program on the device, prints the
# output, and kills the app if interrupted.

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source $SCRIPT_DIR/utils/android_setup.sh
source $SCRIPT_DIR/utils/setup_adb.sh

if [ ! -f "${SKIA_OUT}/${APP_ARGS[0]}" ];
then
  echo "Unable to find ${APP_ARGS[0]} executable"
  exit 1
fi

verbose "pushing binaries onto the device..."
adb_push_if_needed "${SKIA_OUT}/${APP_ARGS[0]}" /data/local/tmp
if [[ -n $RESOURCE_PATH ]]; then
  verbose "pushing resources onto the device..."
  adb_push_if_needed "${SCRIPT_DIR}/../../../resources" $RESOURCE_PATH
fi

if [ $LOGCAT ]; then
   verbose "clearing the device logs..."
  $ADB $DEVICE_SERIAL logcat -c;
fi
STATUS_FILENAME="/data/local/tmp/.skia_tmp_$(date +%s%N)"
CMD_FILENAME=".skia_cmd_tmp_$(date +%s%N)"
echo "/data/local/tmp/${APP_ARGS[*]}; \
     echo \$? > ${STATUS_FILENAME}" > ${CMD_FILENAME}
chmod +x ${CMD_FILENAME}
verbose "======== To reproduce this run: ========"
verbose "android_run_skia ${APP_ARGS[*]}"
verbose "========================================"
verbose "pushing command file onto the device..."
$ADB ${DEVICE_SERIAL} push ${CMD_FILENAME} /data/local/tmp
rm ${CMD_FILENAME}
verbose "preparing to run ${APP_ARGS[0]} on the device..."
$ADB ${DEVICE_SERIAL} shell sh /data/local/tmp/${CMD_FILENAME}

if [ -z "$($ADB $DEVICE_SERIAL shell 'if [ -f $STATUS_FILENAME ]; then echo exists; fi')" ]; then
  if [ $LOGCAT ]; then $ADB $DEVICE_SERIAL logcat -d; fi
  echo "***********************************************************************"
  echo "The application terminated unexpectedly and did not produce an exit code"
  echo "***********************************************************************"
  exit 1
fi

EXIT_CODE=`$ADB ${DEVICE_SERIAL} shell cat ${STATUS_FILENAME}`
$ADB ${DEVICE_SERIAL} shell rm -f ${STATUS_FILENAME} ${CMD_FILENAME}

# check to see if the 'cat' command failed and print errors accordingly
if [[ ${EXIT_CODE} == *${STATUS_FILENAME}* ]]; then
  if [ $LOGCAT ]; then $ADB $DEVICE_SERIAL logcat -d; fi
  echo "***********************************************************************"
  echo "ADB failed to retrieve the application's exit code"
  echo "***********************************************************************"
  exit 1
fi

echo "EXIT_CODE is ${EXIT_CODE}"
if [[ "${EXIT_CODE}" != 0* ]]; then
  if [ $LOGCAT ]; then $ADB $DEVICE_SERIAL logcat -d; fi
  exit 1
fi
exit 0
