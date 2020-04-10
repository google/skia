#!/bin/bash
# Copyright 2019 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This assumes it is being run inside a docker container with the following
# mounts:
#  /SRC: Swarming start dir
#  /OUT: output directory for gn and ninja

set -e
set -x

export PATH="/SRC/recipe_bundle/depot_tools:${PATH}"

cd /SRC/cache/work/flutter/src
flutter/tools/gn "$@"
# This is the same as /OUT, but necessary since GN sets a command to do a
# version check via relative path (../../flutter/third_party/gn/gn)
ninja -C /SRC/cache/work/flutter/src/out/android_release -j100
