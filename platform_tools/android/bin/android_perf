#!/bin/bash
#
# android_perf: utility for running perf on an android device
#
# The basic usage sequence is to run...
#  1) perf record [gm/tests/bench] # runs profiler on specified app
#  2) perf report # prints profiler results
#  3) perf clean # cleans the temporary directory used to store results
#

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source $SCRIPT_DIR/utils/android_setup.sh
source $SCRIPT_DIR/utils/setup_adb.sh

# grab and remove the perf command from the input args
PERF_CMD=${APP_ARGS[0]}
unset APP_ARGS[0]
runVars=("${APP_ARGS[@]}")  # shift array indices

# We need the debug symbols from these files
PERF_TMP_DIR=$(pwd)/android_perf_tmp

TMP_SYS_BIN=$PERF_TMP_DIR/system/bin
TMP_SYS_LIB=$PERF_TMP_DIR/system/lib
TMP_APP_LOC=$PERF_TMP_DIR/data/local/tmp

perf_setup() {

    mkdir -p $TMP_SYS_BIN
    mkdir -p $TMP_SYS_LIB
    mkdir -p $TMP_APP_LOC

    echo "Copying symbol files"
    adb_pull_if_needed /system/lib/libc.so $TMP_SYS_LIB
    adb_pull_if_needed /system/lib/libstdc++.so $TMP_SYS_LIB
    adb_pull_if_needed /system/lib/libGLESv2.so $TMP_SYS_LIB
    adb_pull_if_needed /system/lib/libandroid.so $TMP_SYS_LIB
    adb_pull_if_needed /system/lib/libm.so $TMP_SYS_LIB
    adb_pull_if_needed /system/lib/libz.so $TMP_SYS_LIB

    # SKIA_OUT variable is set by android_setup.sh
    if [ ! -f "${SKIA_OUT}/${runVars[0]}" ];
    then
      echo "Unable to find the ${runVars[0]} executable"
      exit 1
    fi

    echo "Pushing simpleperf..."
    adb_push_if_needed $SKIA_OUT/simpleperf /data/local/tmp

    echo "Pushing app..."
    adb_push_if_needed "${SKIA_OUT}/${runVars[0]}" /data/local/tmp
    cp "${SKIA_OUT}/${runVars[0]}" $TMP_APP_LOC
}

perf_record() {

    echo "Killing any running Skia processes."
    $ADB shell ps | grep ${runVars[0]} | awk '{print $2}' | xargs $ADB shell kill

    echo "Starting application"
    $ADB shell /data/local/tmp/${runVars[@]} &

    # WE REALLY REALLY WANT TO BE ABLE TO PASS THE SKIA_LAUNCHER APP DIRECTLY TO
    # PERF, BUT AT THIS POINT THE DATA FILE WE GET WHEN GOING THAT ROUTE IS UNABLE
    # TO BE READ BY THE REPORTING TOOL
    echo "Starting profiler"
    APP_PID=$($ADB shell ps | grep ${runVars[0]} | awk '{print $2}')
    $ADB shell /data/local/tmp/simpleperf record -p ${APP_PID} -o /data/local/tmp/perf.data sleep 70

    $ADB pull /data/local/tmp/perf.data $PERF_TMP_DIR/perf.data

    exit 0;
}

perf_report() {
    adb_pull_if_needed /data/local/tmp/perf.data $PERF_TMP_DIR/perf.data
    $SKIA_OUT/perfhost_report.py -i $PERF_TMP_DIR/perf.data --symfs=$PERF_TMP_DIR ${runVars[@]}
}

# Clean up
perf_clean() {
    rm -rf $PERF_TMP_DIR
}

case $PERF_CMD in
  setup)
      perf_setup ${runVars[@]}
      ;;
  record)
      perf_setup ${runVars[@]}
      perf_record ${runVars[@]}
      ;;
  report)
      perf_report
      ;;
  clean)
      perf_clean
      ;;
    *)
      echo -n "ERROR: unknown perf command ($PERF_CMD), valid values: "
      echo "setup, record, report, clean"
      exit 1;
      ;;
esac

exit 0;
