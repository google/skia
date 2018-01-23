#! /bin/sh

# Copyright 2018 Google Inc.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -e -x

cd "$(dirname "$0")/../.."

BUILD=out/default

python tools/git-sync-deps

bin/gn gen $BUILD

ninja -C $BUILD list_gms list_gpu_unit_tests

DIR=platform_tools/android/apps/skqp/src/main/assets/skqp

mkdir -p $DIR

$BUILD/list_gms > $DIR/KnownGMs.txt

$BUILD/list_gpu_unit_tests > $DIR/KnownGpuUnitTests.txt

