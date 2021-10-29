/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef tessellate_Tessellation_DEFINED
#define tessellate_Tessellation_DEFINED

#include "include/core/SkTypes.h"
#include "include/private/SkVx.h"

class SkMatrix;
class SkPath;
struct SkRect;

namespace skgpu {

struct VertexWriter;

// Don't allow linearized segments to be off by more than 1/4th of a pixel from the true curve.
SK_MAYBE_UNUSED constexpr static float kTessellationPrecision = 4;

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

SK_MAYBE_UNUSED SK_ALWAYS_INLINE float dot(float2 a, float2 b) {
    float2 ab = a*b;
    return ab.x() + ab.y();
}

SK_MAYBE_UNUSED SK_ALWAYS_INLINE float cross(float2 a, float2 b) {
    float2 x = a * b.yx();
    return x[0] - x[1];
}

SK_MAYBE_UNUSED constexpr SK_ALWAYS_INLINE float pow2(float x) { return x*x; }
SK_MAYBE_UNUSED constexpr SK_ALWAYS_INLINE float pow4(float x) { return pow2(x*x); }

// Don't tessellate paths that might have an individual curve that requires more than 1024 segments.
// (See wangs_formula::worst_case_cubic). If this is the case, call "PreChopPathCurves" first.
constexpr static float kMaxTessellationSegmentsPerCurve SK_MAYBE_UNUSED = 1024;

// Returns a new path, equivalent to 'path' within the given viewport, whose verbs can all be drawn
// with 'maxSegments' tessellation segments or fewer. Curves and chops that fall completely outside
// the viewport are flattened into lines.
SkPath PreChopPathCurves(const SkPath&, const SkMatrix&, const SkRect& viewport);

// Writes out the path's inner fan using a middle-out topology. Writes 3 points per triangle.
// Additionally writes out "pad32Count" repetitions of "pad32Value" after each triangle. Set
// pad32Count to 0 if the triangles are to be tightly packed.
VertexWriter WritePathMiddleOutInnerFan(VertexWriter&&,
                                        int pad32Count,
                                        uint32_t pad32Value,
                                        const SkMatrix&,
                                        const SkPath&,
                                        int* numTrianglesWritten);

}  // namespace skgpu

#endif  // tessellate_Tessellation_DEFINED
