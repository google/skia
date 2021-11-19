/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef tessellate_Tessellation_DEFINED
#define tessellate_Tessellation_DEFINED

#include "include/core/SkStrokeRec.h"
#include "include/gpu/GrTypes.h"
#include "include/private/SkVx.h"

class SkMatrix;
class SkPath;
struct SkRect;

namespace skgpu {

struct VertexWriter;

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

// Don't allow linearized segments to be off by more than 1/4th of a pixel from the true curve.
SK_MAYBE_UNUSED constexpr static float kTessellationPrecision = 4;

// Optional attribs that are included in tessellation patches, following the control points and in
// the same order as they appear here.
enum class PatchAttribs {
    // Attribs.
    kNone = 0,
    kFanPoint = 1 << 0,  // [float2] Used by wedges. This is the center point the wedges fan around.
    kStrokeParams = 1 << 1,  // [float2] Used when strokes have different widths or join types.
    kColor = 1 << 2,  // [ubyte4 or float4] Used when patches have different colors.
    kExplicitCurveType = 1 << 3,  // [float] Used when GPU can't infer curve type based on infinity.

    // Extra flags.
    kWideColorIfEnabled = 1 << 4,  // If kColor is set, specifies it to be float4 wide color.
};

GR_MAKE_BITFIELD_CLASS_OPS(PatchAttribs)

// We encode all of a join's information in a single float value:
//
//     Negative => Round Join
//     Zero     => Bevel Join
//     Positive => Miter join, and the value is also the miter limit
//
static float GetJoinType(const SkStrokeRec& stroke) {
    switch (stroke.getJoin()) {
        case SkPaint::kRound_Join: return -1;
        case SkPaint::kBevel_Join: return 0;
        case SkPaint::kMiter_Join: SkASSERT(stroke.getMiter() >= 0); return stroke.getMiter();
    }
    SkUNREACHABLE;
}

// This float2 gets written out with each patch/instance if PatchAttribs::kStrokeParams is enabled.
struct StrokeParams {
    StrokeParams() = default;
    StrokeParams(const SkStrokeRec& stroke) {
        this->set(stroke);
    }
    void set(const SkStrokeRec& stroke) {
        fRadius = stroke.getWidth() * .5f;
        fJoinType = GetJoinType(stroke);
    }
    static bool StrokesHaveEqualParams(const SkStrokeRec& a, const SkStrokeRec& b) {
        return a.getWidth() == b.getWidth() && a.getJoin() == b.getJoin() &&
               (a.getJoin() != SkPaint::kMiter_Join || a.getMiter() == b.getMiter());
    }
    float fRadius;
    float fJoinType;  // See GetJoinType().
};

// When PatchAttribs::kExplicitCurveType is set, these are the values that tell the GPU what type of
// curve is being drawn.
constexpr static float kCubicCurveType SK_MAYBE_UNUSED = 0;
constexpr static float kConicCurveType SK_MAYBE_UNUSED = 1;
constexpr static float kTriangularConicCurveType SK_MAYBE_UNUSED = 2;  // Conic curve with w=Inf.

// Returns the packed size in bytes of the attribs portion of tessellation patches (or instances) in
// GPU buffers.
constexpr size_t PatchAttribsStride(PatchAttribs attribs) {
    return (attribs & PatchAttribs::kFanPoint ? sizeof(float) * 2 : 0) +
           (attribs & PatchAttribs::kStrokeParams ? sizeof(float) * 2 : 0) +
           (attribs & PatchAttribs::kColor
                    ? (attribs & PatchAttribs::kWideColorIfEnabled ? sizeof(float)
                                                                   : sizeof(uint8_t)) * 4 : 0) +
           (attribs & PatchAttribs::kExplicitCurveType ? sizeof(float) : 0);
}

// Don't tessellate paths that might have an individual curve that requires more than 1024 segments.
// (See wangs_formula::worst_case_cubic). If this is the case, call "PreChopPathCurves" first.
constexpr static float kMaxTessellationSegmentsPerCurve SK_MAYBE_UNUSED = 1024;

// Returns a new path, equivalent to 'path' within the given viewport, whose verbs can all be drawn
// with 'maxSegments' tessellation segments or fewer, while staying within '1/tessellationPrecision'
// pixels of the true curve. Curves and chops that fall completely outside the viewport are
// flattened into lines.
SkPath PreChopPathCurves(float tessellationPrecision,
                         const SkPath&,
                         const SkMatrix&,
                         const SkRect& viewport);

// Finds 0, 1, or 2 T values at which to chop the given curve in order to guarantee the resulting
// cubics are convex and rotate no more than 180 degrees.
//
//   - If the cubic is "serpentine", then the T values are any inflection points in [0 < T < 1].
//   - If the cubic is linear, then the T values are any 180-degree cusp points in [0 < T < 1].
//   - Otherwise the T value is the point at which rotation reaches 180 degrees, iff in [0 < T < 1].
//
// 'areCusps' is set to true if the chop point occurred at a cusp (within tolerance), or if the chop
// point(s) occurred at 180-degree turnaround points on a degenerate flat line.
int FindCubicConvex180Chops(const SkPoint[], float T[2], bool* areCusps);

// Returns true if the given conic (or quadratic) has a cusp point. The w value is not necessary in
// determining this. If there is a cusp, it can be found at the midtangent.
inline bool ConicHasCusp(const SkPoint p[3]) {
    SkVector a = p[1] - p[0];
    SkVector b = p[2] - p[1];
    // A conic of any class can only have a cusp if it is a degenerate flat line with a 180 degree
    // turnarund. To detect this, the beginning and ending tangents must be parallel
    // (a.cross(b) == 0) and pointing in opposite directions (a.dot(b) < 0).
    return a.cross(b) == 0 && a.dot(b) < 0;
}

}  // namespace skgpu

#endif  // tessellate_Tessellation_DEFINED
