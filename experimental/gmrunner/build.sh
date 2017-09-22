#!/bin/bash

# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -x -e

# TODO(stephana): This will move to a better spot, i.e. the app gradle file.
# Builds skia and gmrunner for android.

declare -A BUILD_IDS
BUILD_IDS[arm64]=arm64-v8a
BUILD_IDS[arm]=armeabi
BUILD_IDS[x64]=x86_64
BUILD_IDS[x86]=x86
BUILD_IDS[mipsel]=mips
BUILD_IDS[mips64el]=mips64

LIBS_DIR=experimental/CTS18/app/libs
SKIA_ROOT=../..

# Change to skia root and build all versions.
pushd $SKIA_ROOT
python ./tools/git-sync-deps
for BUILD_ID in ${!BUILD_IDS[@]}; do
  OUT_DIR=out/${BUILD_ID}
  ./bin/gn gen ${OUT_DIR} --args='is_debug=true ndk="/usr/local/google/home/stephana/Android/Sdk/ndk-bundle" target_cpu="'"$BUILD_ID"'"'
  ninja -C ${OUT_DIR}

  echo ${BUILD_IDS[$BUILD_ID]}
  TARGET_DIR=${LIBS_DIR}/${BUILD_IDS[$BUILD_ID]}
  mkdir -p ${TARGET_DIR}
  echo $TARGET_DIR

  cp ${OUT_DIR}/libskia.a ${TARGET_DIR}
  cp ${OUT_DIR}/libgmrunner.so ${TARGET_DIR}
done
popd
