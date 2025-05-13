#!/bin/bash
# Copyright 2022 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Find this file and look for ../../external
BASE_DIR=$( realpath $( dirname ${BASH_SOURCE[0]}))
CLANG_DIR=$( dirname $( dirname $BASE_DIR))/external/*clang_mac

$CLANG_DIR/bin/llvm-ar $@
