/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_tessellate_Tessellation_DEFINED
#define skgpu_tessellate_Tessellation_DEFINED

#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkStrokeRec.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkMacros.h"

#include <math.h>
#include <algorithm>
#include <cstddef>
#include <cstdint>

class SkMatrix;
class SkPath;
struct SkRect;

namespace skgpu::tess {

// Don't allow linearized segments to be off by more than 1/4th of a pixel from the true curve.
constexpr static float kPrecision = 4;

// This is the maximum number of subdivisions of a Bezier curve that can be represented in the fixed
// count vertex and index buffers. If rendering a curve that requires more subdivisions, it must be
// chopped.
constexpr static int kMaxResolveLevel = 5;

// This is the maximum number of parametric segments (linear sections) that a curve can be split
// into. This is the same for path filling and stroking, although fixed-count stroking also uses
// additional vertices to handle radial segments, joins, and caps. Additionally the fixed-count
// path filling algorithms snap their dynamic vertex counts to powers-of-two, whereas the stroking
// algorithm does not.
constexpr static int kMaxParametricSegments = 1 << kMaxResolveLevel;
constexpr static int kMaxParametricSegments_p2 = kMaxParametricSegments * kMaxParametricSegments;
constexpr static int kMaxParametricSegments_p4 = kMaxParametricSegments_p2 *
                                                 kMaxParametricSegments_p2;

// Don't tessellate paths that might have an individual curve that requires more than 1024 segments.
// (See wangs_formula::worst_case_cubic). If this is the case, call "PreChopPathCurves" first.
// Standard chopping, when Wang's formula is between kMaxParametricSegments and
// kMaxTessellationSegmentsPerCurve is handled automatically by PatchWriter. It differs from
// PreChopPathCurves in that it does no culling of offscreen chopped paths.
constexpr static float kMaxSegmentsPerCurve = 1024;
constexpr static float kMaxSegmentsPerCurve_p2 = kMaxSegmentsPerCurve * kMaxSegmentsPerCurve;
constexpr static float kMaxSegmentsPerCurve_p4 = kMaxSegmentsPerCurve_p2 * kMaxSegmentsPerCurve_p2;

// Returns a new path, equivalent to 'path' within the given viewport, whose verbs can all be drawn
// with 'maxSegments' tessellation segments or fewer, while staying within '1/tessellationPrecision'
// pixels of the true curve. Curves and chops that fall completely outside the viewport are
// flattened into lines.
SkPath PreChopPathCurves(float tessellationPrecision,
                         const SkPath&,
                         const SkMatrix&,
                         const SkRect& viewport);

// How many triangles are in a curve with 2^resolveLevel line segments?
// Resolve level defines the tessellation factor for filled paths drawn using curves or wedges.
constexpr static int NumCurveTrianglesAtResolveLevel(int resolveLevel) {
    // resolveLevel=0 -> 0 line segments -> 0 triangles
    // resolveLevel=1 -> 2 line segments -> 1 triangle
    // resolveLevel=2 -> 4 line segments -> 3 triangles
    // resolveLevel=3 -> 8 line segments -> 7 triangles
    // ...
    return (1 << resolveLevel) - 1;
}

// Optional attribs that are included in tessellation patches, following the control points and in
// the same order as they appear here.
enum class PatchAttribs {
    // Attribs.
    kNone = 0,
    kJoinControlPoint = 1 << 0, // [float2] Used by strokes. This defines tangent direction.
    kFanPoint = 1 << 1,  // [float2] Used by wedges. This is the center point the wedges fan around.
    kStrokeParams = 1 << 2,  // [float2] Used when strokes have different widths or join types.
    kColor = 1 << 3,  // [ubyte4 or float4] Used when patches have different colors.
    kPaintDepth = 1 << 4, // [float] Used in Graphite to specify depth attachment value for draw.
    kExplicitCurveType = 1 << 5,  // [float] Used when GPU can't infer curve type based on infinity.
    kSsboIndex = 1 << 7,  // [int] Used to index into a shared storage buffer for this patch's
                          //       uniform values.

    // Extra flags.
    kWideColorIfEnabled = 1 << 6,  // If kColor is set, specifies it to be float4 wide color.
};

SK_MAKE_BITFIELD_CLASS_OPS(PatchAttribs)

// When PatchAttribs::kExplicitCurveType is set, these are the values that tell the GPU what type of
// curve is being drawn.
constexpr static float kCubicCurveType [[maybe_unused]] = 0;
constexpr static float kConicCurveType [[maybe_unused]] = 1;
constexpr static float kTriangularConicCurveType [[maybe_unused]] = 2;  // Conic curve with w=Inf.

// Returns the packed size in bytes of the attribs portion of tessellation patches (or instances) in
// GPU buffers.
constexpr size_t PatchAttribsStride(PatchAttribs attribs) {
    return (attribs & PatchAttribs::kJoinControlPoint ? sizeof(float) * 2 : 0) +
           (attribs & PatchAttribs::kFanPoint ? sizeof(float) * 2 : 0) +
           (attribs & PatchAttribs::kStrokeParams ? sizeof(float) * 2 : 0) +
           (attribs & PatchAttribs::kColor
                    ? (attribs & PatchAttribs::kWideColorIfEnabled ? sizeof(float)
                                                                   : sizeof(uint8_t)) * 4 : 0) +
           (attribs & PatchAttribs::kPaintDepth ? sizeof(float) : 0) +
           (attribs & PatchAttribs::kExplicitCurveType ? sizeof(float) : 0) +
           (attribs & PatchAttribs::kSsboIndex ? (sizeof(uint32_t) * 2) : 0);
}
constexpr size_t PatchStride(PatchAttribs attribs) {
    return 4*sizeof(SkPoint) + PatchAttribsStride(attribs);
}

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

// We encode all of a join's information in a single float value:
//
//     Negative => Round Join
//     Zero     => Bevel Join
//     Positive => Miter join, and the value is also the miter limit
//
inline float GetJoinType(const SkStrokeRec& stroke) {
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
    StrokeParams(float radius, float joinType) : fRadius(radius), fJoinType(joinType) {}
    StrokeParams(const SkStrokeRec& stroke) {
        this->set(stroke);
    }
    void set(const SkStrokeRec& stroke) {
        fRadius = stroke.getWidth() * .5f;
        fJoinType = GetJoinType(stroke);
    }

    float fRadius;
    float fJoinType;  // See GetJoinType().
};

inline bool StrokesHaveEqualParams(const SkStrokeRec& a, const SkStrokeRec& b) {
    return a.getWidth() == b.getWidth() && a.getJoin() == b.getJoin() &&
            (a.getJoin() != SkPaint::kMiter_Join || a.getMiter() == b.getMiter());
}

// Returns the fixed number of edges that are always emitted with the given join type. If the
// join is round, the caller needs to account for the additional radial edges on their own.
// Specifically, each join always emits:
//
//   * Two colocated edges at the beginning (a full-width edge to seam with the preceding stroke
//     and a half-width edge to begin the join).
//
//   * An extra edge in the middle for miter joins, or else a variable number of radial edges
//     for round joins (the caller is responsible for counting radial edges from round joins).
//
//   * A half-width edge at the end of the join that will be colocated with the first
//     (full-width) edge of the stroke.
//
constexpr int NumFixedEdgesInJoin(SkPaint::Join joinType) {
    switch (joinType) {
        case SkPaint::kMiter_Join:
            return 4;
        case SkPaint::kRound_Join:
            // The caller is responsible for counting the variable number of middle, radial
            // segments on round joins.
            [[fallthrough]];
        case SkPaint::kBevel_Join:
            return 3;
    }
    SkUNREACHABLE;
}
constexpr int NumFixedEdgesInJoin(const StrokeParams& strokeParams) {
    // The caller is responsible for counting the variable number of segments for round joins.
    return strokeParams.fJoinType > 0.f ? /* miter */ 4 : /* round or bevel */ 3;
}

// Decides the number of radial segments the tessellator adds for each curve. (Uniform steps
// in tangent angle.) The tessellator will add this number of radial segments for each
// radian of rotation in local path space.
inline float CalcNumRadialSegmentsPerRadian(float approxDevStrokeRadius) {
    float cosTheta = 1.f - (1.f / kPrecision) / approxDevStrokeRadius;
    return .5f / acosf(std::max(cosTheta, -1.f));
}

}  // namespace skgpu::tess

#endif  // skgpu_tessellate_Tessellation_DEFINED
