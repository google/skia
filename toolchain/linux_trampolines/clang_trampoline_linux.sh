#!/bin/bash
# Copyright 2021 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Find this file and look for ../../external
BASE_DIR=$( realpath $( dirname ${BASH_SOURCE[0]}))
CLANG_DIR=$( dirname $( dirname $BASE_DIR))/external/*clang_linux_amd64

export LD_LIBRARY_PATH=$CLANG_DIR"/usr/lib/x86_64-linux-gnu"

$CLANG_DIR/bin/clang $@
