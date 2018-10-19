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
if [ "$#" -gt 1 ]; then
    ARGS="-e class org.skia.skqp.SkQPRunner#${1}"
    shift
    for arg; do
        ARGS="${ARGS},org.skia.skqp.SkQPRunner#${arg}"
    done
fi

TDIR="$(mktemp -d "${TMPDIR:-/tmp}/skqp_report.XXXXXXXXXX")"

filter() {
    local re='^.*org\.skia\.skqp: output written to "\([^"]*\)".*$'
    while IFS='' read -r line ; do
        if printf '%s\n' "$line" | grep -q "$re"; then
            D="$(printf '%s\n' "$line" | sed -n "s/${re}/\1/p")"
            echo "$D" > "${TDIR}/loc"
        fi
        printf '%s\n' "$line" | sed 's/^[0-9-]\+ [0-9.:]\+ [0-9]\+ [0-9]\+//'
    done
}

adb install -r "$APK" || exit 2
adb logcat -c

adb logcat TestRunner org.skia.skqp skia DEBUG "*:S" | tee "${TDIR}/logcat.txt" | filter &
LOGCAT_PID=$!

ADBSHELL_PID=''
trap 'kill $LOGCAT_PID; kill $ADBSHELL_PID' INT

adb shell am instrument $ARGS -w org.skia.skqp \
    >  "${TDIR}/stdout.txt" \
    2> "${TDIR}/stderr.txt" &
ADBSHELL_PID=$!

wait $ADBSHELL_PID
trap - INT
kill $LOGCAT_PID

printf '\nTEST OUTPUT IS IN: "%s"\n\n' "$TDIR"

if ! [ -f "${TDIR}/loc" ]; then exit 2; fi

ODIR="$(cat "${TDIR}/loc")/skqp_report"

if ! adb shell "test -d '$ODIR'" ; then
    echo 'missing output :('
    exit 3
fi

adb pull "${ODIR}/out.csv" "${ODIR}/report.html" "${ODIR}/images" "${TDIR}/"
REPORT="$TDIR/report.html"
grep 'f(.*;' "$REPORT"
echo "$REPORT"
case "$(uname)" in
    Linux)
        [ "$DISPLAY" ] && xdg-open "$REPORT" &
        sleep 1
        ;;
    Darwin)
        open "$REPORT" &
        ;;
esac

