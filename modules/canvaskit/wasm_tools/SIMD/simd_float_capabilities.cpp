// Copyright 2020 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "include/private/SkVx.h"
#include <emscripten.h>
#include <stdio.h>

// How to read this file:
// - Lines with "//GOOD" are compatible with WASM SIMD and are automatically compiled
//   into WASM SIMD operations by emscripten.
// - Lines with "//N/A" are not operations that are compatible with this type of data.
// - Lines with "GOOD (FIXED)" are compatible with WASM SIMD but are NOT automatically
//   compiled into WASM SIMD operations by emscripten. Special WASM SIMD intrinsics have been
//   specified in skia/include/private/SkVx.h to tell emscripten how to compile them to WASM SIMD
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
// 3. Run `./build_simd_test.sh simd_float_capabilities.cpp` to build and output WASM SIMD operations
//   present in the compiled WASM.
// 4. Read the output in the console to see if the WASM SIMD operations you expected were present in
//   the resulting compiled WASM.

int main() {
  auto vec1 = skvx::Vec<4, float>({11.f, -22.f, 33.f, -44.f});
  auto vec2 = skvx::Vec<4, float>({-.5f, 100.5f, 100.5f, -.5f});

  //auto vec3 = skvx::join(vec1, vec2); //not available in wasm
  // note: may be possible using "widening"

  //vec1 = vec1 + vec2; //GOOD
  //vec1 = vec1 - vec2; //GOOD
  //vec1 = vec1 * vec2; //GOOD
  //vec1 = vec1 / vec2; //GOOD

  //vec1 = vec1 ^ vec2; //N/A
  //vec1 = vec1 & vec2; //N/A
  //vec1 = vec1 | vec2; //N/A

  //vec1 = !vec1; //N/A
  //vec1 = -vec1; //GOOD
  //vec1 = ~vec1; //N/A

  //vec1 = vec1 << 2; //N/A
  //vec1 = vec1 >> 2; //N/A

  //auto vec3 = vec1 == vec2; //GOOD
  //auto vec3  = vec1 != vec2; //GOOD
  //auto vec3 = vec1 <= vec2; //GOOD
  //auto vec3 = vec1 >= vec2; //GOOD
  //auto vec3 = vec1 < vec2; //GOOD
  //auto vec3 = vec1 > vec2; //GOOD

  //auto vec3 = skvx::any(vec1); //N/A
  //auto vec3 = skvx::all(vec1); //N/A

  //vec1 = skvx::max(vec1, vec2); //GOOD (FIXED)
  //vec1 = skvx::min(vec1, vec2); //GOOD (FIXED)

  //vec1 = skvx::pow(vec1, vec2); //not available in wasm
  //vec1 = skvx::atan(vec1); //not available in wasm
  //vec1 =  ceil(vec1); //not available in wasm, note: maybe could use "comparisons"
  //vec1 = skvx::floor(vec1); //not available in wasm
  //vec1 = skvx::trunc(vec1); //not available in wasm
  // note: maybe possible using trunc_sat_f32x4_s and convert_i32x4_s?
  //vec1 = skvx::round(vec1); //not available in wasm
  // note: maybe possible using trunc_sat_f32x4_s and convert_i32x4_s?
  //vec1 = skvx::sqrt(vec1); //GOOD (FIXED)
  //vec1 = skvx::abs(vec1); //GOOD (FIXED)
  //vec1 = skvx::sin(vec1); //not available in wasm
  //vec1 = skvx::cos(vec1); //not available in wasm
  //vec1 = skvx::tan(vec1); //not available in wasm

  //auto vec3 = skvx::lrint(vec1); //???
  // note: may be possible using f32x4.convert_i32x4_s, would need to test correctness.

  //vec1 = skvx::rcp(vec1); //GOOD (FIXED) previous: N/A-BAD, doesn't use SIMD div
  //vec1 = skvx::rsqrt(vec1); //GOOD (FIXED) previous: BAD, doesn't use SIMD sqrt or div

  //vec1 = skvx::if_then_else(vec1, vec1, vec2); //N/A

  //vec1 = skvx::shuffle<2,1,0,3>(vec1); //GOOD

  //vec1 = skvx::fma(vec1, vec2, vec1); //not available in wasm (no fused multiply-add is available)
  //vec1 = skvx::fract(vec1); //???

  //printf("result: { %f, %f, %f, %f }\n", vec1[0], vec1[1], vec1[2], vec1[3]);

  return 0;
}
