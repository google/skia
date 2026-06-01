#!/bin/bash
#
# Copyright 2018 Google LLC All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -e

# This script is designed to be used as a cc_wrapper in GN:
#   cc_wrapper = "../../tools/clang-tidy.sh"
# (path relative to the build directory)

# When used as a cc_wrapper, the first argument is the compiler.
REAL_CXX="$1"
shift

# Find clang-tidy relative to the compiler, or via CLANG_TIDY env, or in PATH.
if [ -z "$CLANG_TIDY" ]; then
  TIDY_BIN="$(dirname "$REAL_CXX")/clang-tidy"
  if [ -x "$TIDY_BIN" ]; then
    CLANG_TIDY="$TIDY_BIN"
  else
    CLANG_TIDY="clang-tidy"
  fi
fi

args="$@"
src=""
last_arg=""

for arg in "$@"; do
    if [ "$last_arg" == "-c" ]; then
        src=$arg
        break
    fi
    last_arg=$arg
done

# Ignore third party code (which we don't control). Generally it's found in //third_party/
# but is also found in //src/gpu/vk/vulkanmemoryallocator and in a generated folder when building
# Dawn using CMake
if [ "$src" ] && [[ "$src" != *"third_party"* ]] && [[ "$src" != *"vulkanmemoryallocator"* ]] && [[ "$src" != *"cmake_"* ]]; then
    "$CLANG_TIDY" -quiet -header-filter='.*' -warnings-as-errors='*' "$src" -- "$REAL_CXX" $args
fi

exec "$REAL_CXX" $args

