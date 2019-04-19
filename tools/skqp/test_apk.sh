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

if [ "$#" -gt 0 ]; then
    SKQP_ARGS="-e class org.skia.skqp.SkQPRunner#${1}"
    shift
    for arg; do
        SKQP_ARGS="${SKQP_ARGS},org.skia.skqp.SkQPRunner#${arg}"
    done
    export SKQP_ARGS
fi

TDIR="$(mktemp -d "${TMPDIR:-/tmp}/skqp_report.XXXXXXXXXX")"
THIS="$(dirname "$0")"

sh "$THIS/run_apk.sh" "$APK" "$TDIR"

"$THIS/../../bin/sysopen" "$TDIR"/skqp_report_*/report.html
