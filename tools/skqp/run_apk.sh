#! /bin/sh
# Copyright 2019 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Note:
#   The ANDROID_SERIAL, SKQP_ARGS, and SKQP_SLEEP environment variables affect
#   this script.

if ! [ -f "$1" ] || ! [ -d "$2" ] ; then
  echo "Usage:  $0 SKQP_APK_FILE_PATH RESULTS_DIRECTORY" >&2
  exit 1
fi

SED_CMD='s/^.* org.skia.skqp: output written to "\([^"]*\)".*$/\1/p'
APK="$1"
DST="$2"

printf '\n\nAPK = "%s"\nDST = "%s"\n\n' "$APK" "$DST"

set -x

timeout 60 adb wait-for-device || exit 1

sleep ${SKQP_SLEEP:-0}

adb uninstall org.skia.skqp > /dev/null 2>&1

adb install "$APK" || exit 1

adb logcat -c

adb shell am instrument $SKQP_ARGS -w org.skia.skqp 2>&1 | tee "$DST"/stdout

adb logcat -d TestRunner org.skia.skqp skia DEBUG '*:S' > "$DST"/logcat

ODIR="$(sed -n "$SED_CMD" "$DST"/logcat | head -1)"

if adb shell "test -d '$ODIR'"; then adb pull "$ODIR" "$DST"; fi
