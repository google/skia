#!/bin/bash
# Copyright 2020 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This script takes a path to a .wasm file as input and outputs textual representations of all wasm
# SIMD operations present in the .wasm file.
#
# Example usage: ./simd_test.sh simd_float_capabilities.wasm

# Requires that emscripten and wasm2wat are added to your PATH.
# Requires, and is verified to work with
# - The output of `wasm2wat --version` should be `1.0.13 (1.0.17)`
#   - install from here: https://github.com/WebAssembly/wabt


wasm2wat --enable-simd $1 > output/simd_test.wat

# The following lines output all SIMD operations produced in the output WASM.
# Useful for checking that SIMD instructions are actually being used.
# e.g. for the following C++ code:
#  auto vec1 = skvx::Vec<2, double>({11.f, -22.f}) + skvx::Vec<2, double>({13.f, -1.f});
# it is expected that the f64x2.add operation is present in the output WASM.
echo "The following WASM SIMD operations were used in the compiled code:"
grep -f wasm_simd_types.txt output/simd_test.wat
