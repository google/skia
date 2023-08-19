#!/bin/bash
#
# android_gdb_native: Pushes gdbserver, connects to specified Skia app,
#                     and enters command line debugging environment.

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source $SCRIPT_DIR/utils/android_setup.sh

# setup the gdbserver
$SCRIPT_DIR/android_gdbserver -C ${SKIA_OUT} ${APP_ARGS[@]}

# quit if gdbserver setup failed
if [[ "$?" != "0" ]]; then
  echo "ERROR: gdbserver failed to setup properly."
  exit 1
fi

# Wait for gdbserver
sleep 2

# variables that must match those in gdb_server
GDB_TMP_DIR=$SKIA_OUT/android_gdb_tmp
APP_NAME=${APP_ARGS[0]}
PORT=5039

# Set up gdb commands
GDBSETUP=$GDB_TMP_DIR/gdb.setup
{
    echo "file ${GDB_TMP_DIR}/${APP_NAME}"
    echo "target remote :${PORT}"
    echo "set solib-absolute-prefix ${GDB_TMP_DIR}"
    echo "set solib-search-path ${GDB_TMP_DIR}"

    echo "break main"
    echo "continue"
} > $GDBSETUP


# Launch gdb client
HOST=`uname | tr '[A-Z]' '[a-z]'`
if [ $HOST == "darwin" ]; then
    GDB_HOST=$ANDROID_NDK_HOME/prebuilt/darwin-x86_64/bin/gdb
elif [ $HOST == "linux" ]; then
     GDB_HOST=$ANDROID_NDK_HOME/prebuilt/linux-x86_64/bin/gdb
else
    echo "Could not automatically determine OS!"
    exit 1;
fi

echo "Entering gdb client shell"
$GDB_HOST -x $GDBSETUP

# Clean up:
# We could 'rm -rf $GDB_TMP_DIR', but doing so would cause subsequent debugging
# sessions to take longer than necessary. The tradeoff is to now force the user
# to remove the directory when they are done debugging.
rm $GDBSETUP
