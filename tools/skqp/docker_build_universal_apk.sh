#!/bin/sh
# Copyright 2018 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Note: you may need to run as root for docker permissions.

OUT="$(mktemp -d "${TMPDIR:-/tmp}/skqp_apk.XXXXXXXXXX")"
BUILD="$(mktemp -d "${TMPDIR:-/tmp}/skqp_apk_build.XXXXXXXXXX")"
SKIA_ROOT="$(cd "$(dirname "$0")/../.."; pwd)"

cd "${SKIA_ROOT}/infra/skqp/docker"

docker build -t android-skqp ./android-skqp/
docker run --privileged -d --name android_em \
        -e DEVICE="Samsung Galaxy S6" \
        -v "$SKIA_ROOT":/SRC \
        -v "$OUT":/OUT \
        -v "$BUILD":/BUILD \
        android-skqp
docker exec -it \
    -e SKQP_OUTPUT_DIR=/OUT \
    -e SKQP_BUILD_DIR=/BUILD \
    android_em /SRC/tools/skqp/make_universal_apk.py
docker kill android_em
docker rm android_em
rm -r "$BUILD"
chmod -R 0777 $OUT
ls -l $OUT/*.apk
