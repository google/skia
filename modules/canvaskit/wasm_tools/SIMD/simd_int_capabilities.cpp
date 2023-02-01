// Copyright 2020 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/base/SkVx.h"
#include <emscripten.h>
#include <stdio.h>

// How to read this file:
// - Lines with "//GOOD" are compatible with WASM SIMD and are automatically compiled
//   into WASM SIMD operations by emscripten.
// - Lines with "//N/A" are operations that are not compatible with this type of data.
// - Lines with "GOOD (FIXED)" are compatible with WASM SIMD but are NOT automatically
//   compiled into WASM SIMD operations by emscripten. Special WASM SIMD intrinsics have been
//   specified in skia/src/base/SkVx.h to tell emscripten how to compile them to WASM SIMD
//   operations.
// - Lines with "//not available in wasm" do not have compatible WASM SIMD operations. Emscripten
//   compiles these operations into non-SIMD WASM.
// - Lines with "//???" may be more complex and it is not clear if they have compatible WASM SIMD
//   operations. More work could be needed on these operations.

// How to use this file for testing WASM SIMDification of operations:
// 1. Reference https://github.com/WebAssembly/simd/blob/master/proposals/simd/SIMD.md
//   and https://github.com/llvm/llvm-project/blob/master/clang/lib/Headers/wasm_simd128.h
//   to check if a WASM SIMD operation exists which correspond to any given line of code.
// 2. Uncomment that line of code.
// 3. Run `./build_simd_test.sh simd_int_capabilities.cpp` to build and output WASM SIMD operations present
//   in the compiled WASM.
// 4. Read the output in the console to see if the WASM SIMD operations you expected were present in
//   the resulting compiled WASM.

int main() {
  auto vec1 = skvx::Vec<4, int>({3, 7, 11, 23});
  auto vec2 = skvx::Vec<4, int>({1, 9, 27, 41});

  //auto vec3 = skvx::join(vec1, vec2); //not available in wasm
  // note: may be possible using "widening"

  //vec1 = vec1 + vec2; //GOOD
  //vec1 = vec1 - vec2; //GOOD
  //vec1 = vec1 * vec2; //GOOD
  //vec1 = vec1 / vec2; //N/A

  //vec1 = vec1 ^ vec2; //GOOD
  //vec1 = vec1 & vec2; //GOOD
  //vec1 = vec1 | vec2; //GOOD

  //vec1 = !vec1; //GOOD
  //vec1 = -vec1; //GOOD
  //vec1 = ~vec1; //GOOD

  //vec1 = vec1 << 2; //GOOD
  //vec1 = vec1 >> 2; //GOOD

  //auto vec3 = vec1 == vec2; //GOOD
  //auto vec3  = vec1 != vec2; //GOOD
  //auto vec3 = vec1 <= vec2; //GOOD
  //auto vec3 = vec1 >= vec2; //GOOD
  //auto vec3 = vec1 < vec2; //GOOD
  //auto vec3 = vec1 > vec2; //GOOD

  //auto vec3 = skvx::any(vec1); //GOOD (FIXED)
  //auto vec3 = skvx::all(vec1); //GOOD (FIXED)

  //auto a = skvx::max(vec1, vec2); //GOOD (FIXED)
  //auto a = skvx::min(vec1, vec2); //GOOD (FIXED)

  //vec1 = skvx::pow(vec1, vec2); //not available in wasm
  //vec1 = skvx::atan(vec1); //not available in wasm
  //vec1 = ceil(vec1); //not available in wasm
  //vec1 = skvx::floor(vec1); //not available in wasm
  //vec1 = skvx::trunc(vec1); //N/A
  //vec1 = skvx::round(vec1); //N/A
  //vec1 = skvx::sqrt(vec1); //not available in wasm
  //vec1 = skvx::abs(vec1); //GOOD (FIXED)
  //vec1 = skvx::sin(vec1); //not available in wasm
  //vec1 = skvx::cos(vec1); //not available in wasm
  //vec1 = skvx::tan(vec1); //not available in wasm

  //auto vec3 = skvx::lrint(vec1); //???

  //vec1 = skvx::rcp(vec1); //N/A
  //vec1 = skvx::rsqrt(vec1); //N/A

  //vec1 = skvx::if_then_else(vec1, vec1, vec2); //???

  //vec1 = skvx::shuffle<2,1,0,3>(vec1); //GOOD

  //vec1 = skvx::fma(vec1, vec2, vec1); //not available in wasm (no fused multiply-add is available)
  //vec1 = skvx::fract(vec1); //N/A

  //printf("result: { %i, %i, %i, %i }\n", vec1[0], vec1[1], vec1[2], vec1[3]);

  return 0;
}
