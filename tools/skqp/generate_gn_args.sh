#! /bin/sh

# Copyright 2018 Google Inc.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

BUILD="$1"
ANDROID_NDK="$2"
ARCH="$3"

if [ $# -ne 3 ] || [ -z "$BUILD" ] || ! [ -d "$ANDROID_NDK" ] || [ -z "$ARCH" ]; then
    printf 'Usage:\n  %s TARGET_BUILD_DIR ANDROID_NDK_DIR ARCHITECTURE\n\n' "$0" >&2
    printf 'ARCHITECTURE should be "arm" "arm64" "x86" or "x64"\n\n' >&2
    exit 1
fi

ANDROID_NDK="$(cd "$ANDROID_NDK"; pwd)"

mkdir -p "$BUILD"

cat > "$BUILD/args.gn" << EOF
ndk = "$ANDROID_NDK"
ndk_api = 26
target_cpu = "$ARCH"
skia_embed_resources = true
is_debug = false
skia_enable_pdf = false
EOF


