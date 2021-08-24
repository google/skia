#!/bin/bash
# Copyright 2020 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Call with <swarming dir> <top level of flutter checkout> <out_dir> [GN arguments...]

set -e
set -x

export PATH="$1/recipe_bundle/depot_tools:${PATH}"
# Add CIPD to the PATH for vpython3 and python3.
export PATH="$1/cipd_bin_packages:$1/cipd_bin_packages/bin:$1/cipd_bin_packages/cpython3:$1/cipd_bin_packages/cpython3/bin:${PATH}"
# Set env for vpython and cipd.
export VPYTHON_VIRTUALENV_ROOT=$1/cache/vpython
export CIPD_CACHE_DIR=/tmp/.cipd_cache

# e.g. /mnt/pd0/s/w/ir/cache/work/flutter/src
source_dir=$2
# e.g. /mnt/pd0/s/w/ir/cache/work/flutter/src/out/android_release
out_dir=$3
shift 3
cd $source_dir
flutter/tools/gn "$@"
# This is why we have to mount the entire swarming directory, GN sets a command
# to do a version check via relative path (../../flutter/third_party/gn/gn)
ninja -v -C $out_dir -j100
