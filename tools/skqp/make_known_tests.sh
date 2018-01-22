#! /bin/sh

# Copyright 2018 Google Inc.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -e -x

tools/git-sync-deps
bin/bn gen out/default
ninja -C out/default list_gms list_gpu_unit_tests
out/default/list_gms > \
    platform_tools/android/apps/skqp/src/main/assets/skqp/KnownGMs.txt
out/default/list_gpu_unit_tests > \
    platform_tools/android/apps/skqp/src/main/assets/skqp/KnownGpuUnitTests.txt
git add \
    platform_tools/android/apps/skqp/src/main/assets/skqp/KnownGMs.txt \
    platform_tools/android/apps/skqp/src/main/assets/skqp/KnownGpuUnitTests.txt

