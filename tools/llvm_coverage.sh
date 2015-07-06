#!/bin/sh
# Copyright (c) 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Run from Skia repo like this:
#   $ tools/llvm_coverage.sh dm
# or
#   $ tools/llvm_coverage.sh nanobench

set -x
set -e

make clean
tools/llvm_coverage_build $1
python tools/llvm_coverage_run.py $@
