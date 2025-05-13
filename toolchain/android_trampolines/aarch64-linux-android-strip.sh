#!/bin/bash
# Copyright 2023 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

BASE_DIR=$( realpath $( dirname ${BASH_SOURCE[0]}))
# If we invoke clang with an absolute path, that throws off the #include detection (because it
# will also expect the system paths to be given with an absolute path). Instead, we find out where
# this script is and go two levels up to find external/
cd $( dirname $( dirname $BASE_DIR))
# Then, we can find the toolchain.
CLANG_DIR=external/+download_ndk_linux_amd64_toolchain+ndk_linux_amd64

$CLANG_DIR/toolchains/aarch64-linux-android-4.9/prebuilt/linux-x86_64/bin/aarch64-linux-android-strip $@
