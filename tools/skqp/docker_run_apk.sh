#! /bin/sh
# Copyright 2019 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Notes:
#
#   You may need to run as root for docker permissions.
#
#   The SKQP_ARGS environment variable affects this script.

if ! [ -f "$1" ] ; then
  echo "Usage:  $0 SKQP_APK_FILE_PATH" >&2
  exit 1
fi

APK_DIR="$(cd "$(dirname "$1")"; pwd)"
APK_FILE="$(basename "$1")"
DST="$(mktemp -d "${TMPDIR:-/tmp}/skqp_emulated_test.XXXXXXXXXX")"
SED_CMD='s/^.* org.skia.skqp: output written to "\([^"]*\)".*$/\1/p'
SKQP="$(cd $(dirname "$0"); pwd)"

set -x

cd "${SKQP}/../../infra/skqp/docker"

docker build -t android-skqp ./android-skqp/ > "$DST"/docker-build || exit 2

docker run --privileged --rm -d \
  --name android_em \
  --env=DEVICE="Samsung Galaxy S6" \
  --env=SKQP_SLEEP="30" \
  --env=SKQP_ARGS="$SKQP_ARGS" \
  --volume="$DST":/DST \
  --volume="$APK_DIR":/APK:ro \
  --volume="$SKQP":/SKQP:ro \
  android-skqp > "$DST"/docker-run || exit 3

docker exec android_em sh "/SKQP/run_apk.sh" "/APK/$APK_FILE" "/DST"

docker kill android_em

"${SKQP}/../../bin/sysopen" "$DST"/skqp_report_*/report.html
