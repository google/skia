/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef tessellate_Tessellation_DEFINED
#define tessellate_Tessellation_DEFINED

#include "include/core/SkTypes.h"
#include "include/gpu/GrTypes.h"
#include "include/private/SkVx.h"

class SkMatrix;
class SkPath;
struct SkRect;

namespace skgpu {

struct VertexWriter;

// Don't allow linearized segments to be off by more than 1/4th of a pixel from the true curve.
SK_MAYBE_UNUSED constexpr static float kTessellationPrecision = 4;

// Optional attribs that are included in tessellation patches, following the control points and in
// the same order as they appear here.
enum class PatchAttribs {
    // Attribs.
    kNone = 0,
    kFanPoint = 1 << 0,  // [float2] Used by wedges. This is the center point the wedges fan around.
    kColor = 1 << 1,  // [ubyte4 or float4] Used by direct-rendered convex paths.
    kExplicitCurveType = 1 << 2,  // [float] Used when GPU can't infer curve type based on infinity.

    // Extra flags.
    kWideColorIfEnabled = 1 << 3,  // If kColor is set, specifies it to be float4 wide color.
};

GR_MAKE_BITFIELD_CLASS_OPS(PatchAttribs)

// Returns the packed size in bytes of a tessellation patch (or instance) in GPU buffers.
constexpr size_t PatchStride(PatchAttribs attribs) {
    return sizeof(float) * 8 +  // 4 control points
           (attribs & PatchAttribs::kFanPoint ? sizeof(float) * 2 : 0) +
           (attribs & PatchAttribs::kColor
                    ? (attribs & PatchAttribs::kWideColorIfEnabled ? sizeof(float)
                                                                   : sizeof(uint8_t)) * 4 : 0) +
           (attribs & PatchAttribs::kExplicitCurveType ? sizeof(float) : 0);
}

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

#define AI SK_MAYBE_UNUSED SK_ALWAYS_INLINE

AI float dot(float2 a, float2 b) {
    float2 ab = a*b;
    return ab.x() + ab.y();
}

AI float cross(float2 a, float2 b) {
    float2 x = a * b.yx();
    return x[0] - x[1];
}

// This does not return b when t==1, but it otherwise seems to get better precision than
// "a*(1 - t) + b*t" for things like chopping cubics on exact cusp points.
// The responsibility falls on the caller to check that t != 1 before calling.
template<int N>
AI vec<N> mix(vec<N> a, vec<N> b, vec<N> T) {
    SkASSERT(all((0 <= T) & (T < 1)));
    return (b - a)*T + a;
}

template<int N>
AI vec<N> mix(vec<N> a, vec<N> b, float T) {
    return mix(a, b, vec<N>(T));
}

AI constexpr float pow2(float x) { return x*x; }
AI constexpr float pow4(float x) { return pow2(x*x); }

#undef AI

// Don't tessellate paths that might have an individual curve that requires more than 1024 segments.
// (See wangs_formula::worst_case_cubic). If this is the case, call "PreChopPathCurves" first.
constexpr static float kMaxTessellationSegmentsPerCurve SK_MAYBE_UNUSED = 1024;

// Returns a new path, equivalent to 'path' within the given viewport, whose verbs can all be drawn
// with 'maxSegments' tessellation segments or fewer. Curves and chops that fall completely outside
// the viewport are flattened into lines.
SkPath PreChopPathCurves(const SkPath&, const SkMatrix&, const SkRect& viewport);

}  // namespace skgpu

#endif  // tessellate_Tessellation_DEFINED
