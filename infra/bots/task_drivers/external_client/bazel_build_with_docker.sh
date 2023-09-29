#!/bin/bash
# Copyright 2023 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Run a bazel build command from a specified subdirectory to build a specified target.
# Expects 2 params, the subdirectory and the target.
#
# Why is this needed? We want to be able to test building Skia using our Bazel rules
# with an off-the-shelf compiler and toolchain (and not require clients to use our own).
#
# This assumes it is being run inside a docker container with the following
# mount:
#  /SRC: Base checkout directory
#
# This script has no outputs

set -e
set -x

mkdir /tmp/bazelisk_dl
pushd /tmp/bazelisk_dl
wget -O bazelisk.zip https://chrome-infra-packages.appspot.com/dl/skia/bots/bazelisk_linux_amd64/+/version:0
unzip bazelisk.zip
popd

export PATH="/tmp/bazelisk_dl:${PATH}"

gcc --version
cd /SRC/$1
bazelisk build $2
