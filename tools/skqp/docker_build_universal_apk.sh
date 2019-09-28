#!/bin/sh
# Copyright 2018 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Notes:
#
#    You may need to run as root for docker permissions.
#
#    You *must* run `tools/git-sync-deps` first.

if [ "$SKQP_OUTPUT_DIR" ]; then
    mkdir -p "$SKQP_OUTPUT_DIR" || exit 1
    OUT="$(cd "$SKQP_OUTPUT_DIR"; pwd)"
else
    OUT="$(mktemp -d "${TMPDIR:-/tmp}/skqp_apk.XXXXXXXXXX")"
fi
SKIA_ROOT="$(cd "$(dirname "$0")/../.."; pwd)"

cd "${SKIA_ROOT}/infra/skqp/docker"

docker build -t android-skqp ./android-skqp/

NAME=$(date +android_em_%Y%m%d_%H%M%S)

docker run --rm -d --name "$NAME" \
        --env=DEVICE="Samsung Galaxy S6" \
        --volume="$SKIA_ROOT":/SRC \
        --volume="$OUT":/OUT \
        android-skqp

BUILD="$(docker exec "$NAME" mktemp -d)"

docker exec \
    --env=SKQP_OUTPUT_DIR=/OUT \
    --env=SKQP_BUILD_DIR="$BUILD" \
    "$NAME" /SRC/tools/skqp/make_universal_apk.py

if [ -f "$OUT"/skqp-universal-debug.apk ]; then
    docker exec "$NAME" find /OUT -type f -exec chmod 0666 '{}' '+'
fi

docker kill "$NAME"

ls -l "$OUT"/*.apk 2> /dev/null
