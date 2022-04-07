/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVx_DEFINED
#define GrVx_DEFINED

#include "include/core/SkTypes.h"
#include "include/private/SkVx.h"

// grvx is Ganesh's addendum to skvx, Skia's SIMD library. Here we introduce functions that are
// approximate and/or have LSB differences from platform to platform (e.g., by using hardware FMAs
// when available). When a function is approximate, its error range is well documented and tested.
namespace grvx {

// Use familiar type names and functions from SkSL and GLSL.
template<int N> using vec = skvx::Vec<N, float>;
using float2 = vec<2>;
using float4 = vec<4>;

template<int N> using ivec = skvx::Vec<N, int32_t>;
using int2 = ivec<2>;
using int4 = ivec<4>;

template<int N> using uvec = skvx::Vec<N, uint32_t>;
using uint2 = uvec<2>;
using uint4 = uvec<4>;

static SK_ALWAYS_INLINE float dot(float2 a, float2 b) {
    float2 ab = a*b;
    return ab[0] + ab[1];
}

static SK_ALWAYS_INLINE float cross(float2 a, float2 b) {
    float2 x = a*skvx::shuffle<1,0>(b);
    return x[0] - x[1];
}
};  // namespace grvx

#endif
