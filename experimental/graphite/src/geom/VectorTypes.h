/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_geom_VectorTypes_DEFINED
#define skgpu_geom_VectorTypes_DEFINED

#include "experimental/graphite/include/GraphiteTypes.h"

namespace skgpu {

// Use familiar type names from SkSL.
template<int N> using vec = skvx::Vec<N, float>;
using float2 = vec<2>;
using float4 = vec<4>;

template<int N> using ivec = skvx::Vec<N, int32_t>;
using int2 = ivec<2>;
using int4 = ivec<4>;

template<int N> using uvec = skvx::Vec<N, uint32_t>;
using uint2 = uvec<2>;
using uint4 = uvec<4>;

};  // namespace skgpu

#endif // skgpu_geom_VectorTypes_DEFINED

