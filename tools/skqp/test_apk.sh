#! /bin/sh
# Copyright 2018 Google LLC.
# Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

# If you have more than one device attached, run `adb devices -l` and then set
# the ANDROID_SERIAL environment variable to the correct serial number.

APK="$1"
shift

if ! [ -f "$APK" ]; then
    cat >&2 <<- EOM

	Usage:
	  $0 SKQP_APK_FILE_PATH [OPTIONAL_TESTS_TO_RUN...]

	e.g.:
	  $0 skqp-universal-debug.apk
	or:
	  $0 skqp-universal-debug.apk vk_hairmodes gles_gammatext gles_aarectmodes

	EOM
    exit 1
fi

ARGS=''
if [ "$#" -gt 0 ]; then
    ARGS="-e class org.skia.skqp.SkQPRunner#${1}"
    shift
    for arg; do
        ARGS="${ARGS},org.skia.skqp.SkQPRunner#${arg}"
    done
fi

TDIR="$(mktemp -d "${TMPDIR:-/tmp}/skqp_report.XXXXXXXXXX")"

adb uninstall org.skia.skqp
adb install "$APK" || exit 2
adb logcat -c

adb logcat TestRunner org.skia.skqp skia DEBUG "*:S" | tee "${TDIR}/logcat.txt" &
LOGCAT_PID=$!

ADBSHELL_PID=''
trap 'kill $LOGCAT_PID; kill $ADBSHELL_PID' INT

printf '\n%s\n\n' "adb shell am instrument $ARGS -w org.skia.skqp"
adb shell am instrument $ARGS -w org.skia.skqp \
    >  "${TDIR}/stdout.txt" \
    2> "${TDIR}/stderr.txt" &
ADBSHELL_PID=$!

wait $ADBSHELL_PID
trap - INT
kill $LOGCAT_PID

printf '\nTEST OUTPUT IS IN: "%s"\n\n' "$TDIR"

SED_CMD='s/^.* org.skia.skqp: output written to "\([^"]*\)".*$/\1/p'
ODIR="$(sed -n "$SED_CMD" "${TDIR}/logcat.txt" | head -1)"

if ! adb shell "test -d '$ODIR'" ; then
    echo 'missing output :('
    exit 3
fi

odir_basename="$(basename "$ODIR")"

adb pull "${ODIR}" "${TDIR}/${odir_basename}"

REPORT="${TDIR}/${odir_basename}/report.html"

if [ -f "$REPORT" ]; then
    grep 'f(.*;' "$REPORT"
    echo "$REPORT"
    "$(dirname "$0")"/../../bin/sysopen "$REPORT" > /dev/null 2>&1 &
else
    echo "$TDIR"
fi
