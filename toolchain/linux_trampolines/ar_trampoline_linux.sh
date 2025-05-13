#!/bin/bash
# Copyright 2021 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

if [[ "$@" == *SKIA_SKIP_LINKING* ]]; then
  # The archive file is listed as the second argument to this script, and we must make sure
  # it exists or Bazel will fail a validation step.
  touch $2
  exit 0
fi

# Find this file and look for ../../external
BASE_DIR=$( realpath $( dirname ${BASH_SOURCE[0]}))
CLANG_DIR=$( dirname $( dirname $BASE_DIR))/external/*clang_linux_amd64

$CLANG_DIR/bin/llvm-ar $@
