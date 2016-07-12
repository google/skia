#!/bin/bash
#
# android_gdb_app: Pushes gdbserver, launches Viewer, and connects
# the debugging environment.

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source $SCRIPT_DIR/android_setup.sh "$@"
source $SCRIPT_DIR/utils/setup_adb.sh

APP_ARGS=( "Viewer" ${APP_ARGS[*]} )
PORT=5039

activity="org.skia.viewer/org.skia.viewer.ViewerActivity"
activityShort="org.skia.viewer"

# Forward local to remote socket connection.
$ADB $DEVICE_SERIAL forward "tcp:$PORT" "tcp:$PORT"

# We kill all previous instances of gdbserver to rid all port overriding errors.
if [ $(uname) == "Linux" ]; then
    $ADB $DEVICE_SERIAL shell ps | grep gdbserver | awk '{print $2}' | xargs -r $ADB $DEVICE_SERIAL shell kill
elif [ $(uname) == "Darwin" ]; then
    $ADB $DEVICE_SERIAL shell ps | grep gdbserver | awk '{print $2}' | xargs $ADB $DEVICE_SERIAL shell kill
else
    echo "Could not automatically determine OS!"
    exit 1;
fi

# We need the debug symbols from these files
GDB_TMP_DIR=$SKIA_OUT/android_gdb_tmp
mkdir -p $GDB_TMP_DIR

echo "Pushing gdbserver..."
adb_push_if_needed $ANDROID_TOOLCHAIN/gdbserver /data/local/tmp

# Launch the app
echo "Launching the app..."
$ADB $DEVICE_SERIAL shell "am start -n ${activity} --es cmdLineFlags \"${APP_ARGS[*]:1}\""

# Wait for app process to initialize
sleep 2

# Attach gdbserver to the app process
PID=$($ADB shell ps | grep ${activityShort} | awk '{print $2}')
echo "Attaching to pid: $PID"
$ADB $DEVICE_SERIAL shell /data/local/tmp/gdbserver :$PORT --attach $PID &

# Wait for gdbserver
sleep 2

# Set up gdb commands
GDBSETUP=$GDB_TMP_DIR/gdb.setup
echo "target remote :$PORT" >> $GDBSETUP


# Launch gdb client
echo "Entering gdb client shell"
${ANDROID_TOOLCHAIN}/host_prebuilt/bin/gdb-orig -x $GDBSETUP

# Clean up:
# We could 'rm -rf $GDB_TMP_DIR', but doing so would cause subsequent debugging
# sessions to take longer than necessary. The tradeoff is to now force the user
# to remove the directory when they are done debugging.
rm $GDBSETUP


