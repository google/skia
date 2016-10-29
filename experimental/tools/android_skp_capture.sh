#!/bin/sh

# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

if [ -z "$1" ]; then
    printf 'Usage:\n    android_skp_capture.sh PACKAGE_NAME\n\n'
    printf "Use \`adb shell 'pm list packages'\` to get a listing.\n\n"
    exit 1
fi
if ! command -v adb > /dev/null 2>&1; then
    if [ -x "${ANDROID_SDK_ROOT}/platform-tools/adb" ]; then
        adb() {
            "${ANDROID_SDK_ROOT}/platform-tools/adb" "$@"
        }
    else
        echo 'adb missing'
        exit 2
    fi
fi
phase1_timeout_seconds=15
phase2_timeout_seconds=60
package="$1"
filename="$(date '+%H%M%S').skp"
remote_path="/data/data/${package}/cache/${filename}"
local_path="$(date '+%Y-%m-%d_%H%M%S')_${package}.skp"
key='debug.hwui.capture_frame_as_skp'
adb shell "setprop '${key}' '${remote_path}'"
spin() {
    case "$spin" in
         1) printf '\b|';;
         2) printf '\b\\';;
         3) printf '\b-';;
         *) printf '\b/';;
    esac
    spin=$(( ( ${spin:-0} + 1 ) % 4 ))
    sleep $1
}

banner() {
    printf '\n=====================\n'
    printf '   %s' "$*"
    printf '\n=====================\n'
}
banner '...WAITING...'
adb_test_exist() {
    test '0' = "$(adb shell "test -e \"$1\"; echo \$?")";
}
timeout=$(( $(date +%s) + $phase1_timeout_seconds))
while ! adb_test_exist "$remote_path"; do
    spin 0.05
    if [ $(date +%s) -gt $timeout ] ; then
        printf '\bTimed out.\n'
        adb shell "setprop '${key}' ''"
        exit 3
    fi
done
printf '\b'

banner '...SAVING...'
adb_test_file_nonzero() {
    # grab first byte of `du` output
    X="$(adb shell "du \"$1\" 2> /dev/null | dd bs=1 count=1 2> /dev/null")"
    test "$X" && test "$X" -ne 0
}
#adb_filesize() {
#    adb shell "wc -c \"$1\"" 2> /dev/null | awk '{print $1}'
#}
timeout=$(( $(date +%s) + $phase2_timeout_seconds))
while ! adb_test_file_nonzero "$remote_path"; do
    spin 0.05
    if [ $(date +%s) -gt $timeout ] ; then
        printf '\bTimed out.\n'
        adb shell "setprop '${key}' ''"
        exit 3
    fi
done
printf '\b'

adb shell "setprop '${key}' ''"

i=0; while [ $i -lt 10 ]; do spin 0.10; i=$(($i + 1)); done; echo

adb pull "$remote_path" "$local_path"
if ! [ -f "$local_path" ] ; then
    printf "something went wrong with `adb pull`."
    exit 4
fi
adb shell rm "$remote_path"
printf '\nSKP saved to file:\n    %s\n\n'  "$local_path"
