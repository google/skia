#!/bin/bash
# Copyright 2020 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This script takes a path to a .cpp file, compiles the file to wasm using emscripten, outputs
# textual representations of all wasm SIMD operations present in the compiled .wasm, and starts
# a static file server so that the running .wasm can be manually inspect in a browser.
#
# Example usage: ./build_simd_test.sh simd_float_capabilities.cpp

# Requires that emscripten and wasm2wat are added to your PATH.
# Requires, and is verified to work with
# - The output of `wasm2wat --version` should be `1.0.13 (1.0.17)`
#   - install from here: https://github.com/WebAssembly/wabt
# - emscripten 1.39.16
# - Chrome Canary 86.0.4186.0 with chrome://flags#enable-webassembly-simd enabled

# build the file specified as the first argument with SIMD enabled.
em++ $1 -I ../../../../ -msimd128 -Os -s WASM=1 -o output/simd_test.html
# convert the output WASM to a human readable text format (.wat)
wasm2wat --enable-simd output/simd_test.wasm > output/simd_test.wat

# The following lines output all SIMD operations produced in the output WASM.
# Useful for checking that SIMD instructions are actually being used.
# e.g. for the following C++ code:
#  auto vec1 = skvx::Vec<2, double>({11.f, -22.f}) + skvx::Vec<2, double>({13.f, -1.f});
# it is expected that the f64x2.add operation is present in the output WASM.
echo "The following WASM SIMD operations were used in the compiled code:"
grep -f wasm_simd_types.txt output/simd_test.wat

# Serve the compiled WASM so output can be manually inspected for correctness.
echo "Go check out http://localhost:8000/output/simd_test.html in Chrome Canary 86.0.4186.0 \
or later and enable the chrome://flags#enable-webassembly-simd flag!"
python3 ../../../../tools/serve_wasm.py
