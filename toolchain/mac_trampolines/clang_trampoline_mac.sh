#!/bin/bash
# Copyright 2022 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -euo pipefail

# This script is invoked from two different paths depending on if we are directly compiling
# C++ code (something like /path/to/bazel_cache/sandbox/darwin-sandbox/511/execroot/_main)
# or if the rust compiler is shelling out to the C++ compiler (something like
# /path/to/bazel_cache/sandbox/darwin-sandbox/511/execroot/_main/bazel-out/
#     darwin_arm64-dbg/bin/external/rules_rust++crate+crates__link-cplusplus-1.0.10/
#     _bs.cargo_runfiles/rules_rust++crate+crates__link-cplusplus-1.0.10
# ). We need to invoke clang via a relative path, otherwise the Bazel #include checks get angry
# about an "absolute include filepath" that was not marked as a system include. Thus, we check
# which of the cases we are in to find the clang binary before running it.
if [ -f external/*clang_mac/bin/clang ]; then
external/*clang_mac/bin/clang $@
elif [ -f ../../../../../../../external/*clang_mac/bin/clang ]; then
../../../../../../../external/*clang_mac/bin/clang $@
fi
