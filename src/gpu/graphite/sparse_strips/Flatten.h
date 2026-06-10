/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_Flatten_DEFINED
#define skgpu_graphite_sparse_strips_Flatten_DEFINED

#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/private/SkTDArray.h"
#include "src/core/SkGeometry.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsTypes.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace skgpu::graphite {

class Polyline;

/*
 * This is a translation of the work of the Vello authors:
 * https://raphlinus.github.io/graphics/curves/2019/12/23/flatten-quadbez.html
 * https://github.com/linebender/vello/blob/main/sparse_strips/vello_common/src/flatten.rs
 *
 * The naive approach to flattening a Bézier curve evaluates points at evenly spaced parametric `t`
 * values (e.g., t=0.1, 0.2, ..., 1.0) and recursively subdivides the resulting lines until each
 * segment is within the desired tolerance (https://en.wikipedia.org/wiki/Hausdorff_distance).
 * However, the rate of change in both arc length and curvature is not evenly distributed across
 * `t`. A uniform step in `t` might span a long, mostly flat distance (needing few segments), or it
 * might whip around a tight loop (needing many).
 *
 * Instead, this algorithm maps the quadratic curve into an "arc-length" or uniform-curvature space
 * by projecting it onto a standard y = x^2 parabola. By dividing the curve uniformly in this
 * parabolic space, we can determine the parametric steps required to satisify a given tolerance
 * upfront and without recursion. This allows the exact `t` values to be calculated without data
 * dependencies, increasing ILP and enabling SIMD.
 *
 * 1. `estimate_lines_from_quad`
 *    This function analyzes the control points and transforms the quadratic into the parabolic
 *    space. It calculates the total "parabolic distance" the curve covers. By comparing this
 *    distance against the tolerance, it determines how many subdivsions are required to safely
 *    approximate the curve. It returns a `FlattenParams` struct containing the transformation
 *    constants.
 *
 * 2. `determine_quad_subdiv_t`:
 *    Once we know *how many segments* we need to approximate the curve, we then need to find
 *    *where* to divide it. Using the inverse integral of the parabolic approximation, we map the
 *    `N` evenly spaced parabolic steps *back* into the curve's parametric space (`t`).
 *
 * 3. Culling, Simplification, and Winding:
 *    Curves completely above, below, or to the right of the viewport are culled, producing no
 *    lines. Curves to the left of the viewport are simplified instead of culled because they may
 *    still contribute winding for left-to-right scanline renderers (sparse strips). Curves that
 *    are sufficiently similar to lines are also simplified instead of subdivided. This
 *    simplification avoids subdivision and reduces the number of tiles generated downstream,
 *    while preserving the winding the curve would have otherwise contributed.
 *
 * 4. Lowering Conics:
 *    Conics (rational quadratics) are degree-reduced to standard quadratics using
 *    SkAutoConicToQuads. The curve is recursively chopped in half until the error between the
 *    weighted conic segment and a standard quadratic falls below the tolerance.
 *
 * 5. Lowering Cubics:
 *    Cubics are approximated by a series of quadratics. We estimate the required number of segments
 *    by measuring the curve's geometric deviation from a pure quadratic. The cubic is then sliced
 *    into equal parametric intervals, and the resulting quadratics are subdivided.
 */

class Flatten {
public:
    static constexpr double   kEpsilonD               = 1e-6;
    static constexpr float    kEpsilonF               = static_cast<float>(kEpsilonD);
    static constexpr double   kQuadErrTolerance       = 0.25;
    static constexpr double   kSqrtQuadTolerance      = 0.5;
    static constexpr double   kQuadTolerance2         = kQuadErrTolerance * kQuadErrTolerance;
    // When subdividing a cubic, we have to budget our error between the lowering from Cubics->Quad,
    // and flattenning from Quads->Lines. kCubicAccuracy is the amount we allocate to Cubics->Quad.
    static constexpr float    kCubicAccuracy          = 0.1f;
    static constexpr double   kCubicErrTolerance      = kCubicAccuracy * kQuadErrTolerance;
    static constexpr double   kQuadToLineTolFromCubic = (1.0f - kCubicAccuracy) * kQuadErrTolerance;
    static constexpr float    kQuadSubdivThreshold    = 4.0f * kQuadTolerance2;
    static constexpr float    kCubicSubdivThreshold   = 16.0f / 9.0f * kQuadTolerance2;
    static constexpr uint32_t kMaxQuadsFromCubic      = 16;

    // Since std::sqrt is not constexpr, calculate the value of sqrt(kQuadToLineTolFromCubic)
    // offline. If kCubicAccuracy changes, this needs to be recalculated. static_assert as a
    // reminder.
    static constexpr double   kSqrtQuadFromCubicTol = 0x1.e5b9d136c6d96p-2;
    static_assert(kCubicAccuracy == .1f);

    Flatten() = default;

    template <FlattenMode kMode>
    SK_ALWAYS_INLINE void processPaths(const SkPath& path,
                                       const SkMatrix& ctm,
                                       float width,
                                       float height,
                                       Polyline* polyline) {
        SkASSERT(!ctm.hasPerspective());
        if constexpr (kMode == FlattenMode::kSimd) {
            this->processPathsSimd(path, ctm, width, height, polyline);
        } else {
            this->processPathsScalar(path, ctm, width, height, polyline);
        }
    }

private:
#if defined(GPU_TEST_UTILS)
    template <FlattenMode> friend class FlattenTestRunner;
#endif

    // Buffer for scratch values used during cubic flattening. With the exception of
    // 'fFlattenedCubics', all members are overwritten during flattenCubic*() calls. Consequently,
    // only 'fFlattenedCubics' requires manual state management (clearing) at the start of the
    // function.
    struct CubicFlattenCtx {
        // The on-curve endpoints (p0 and p2) of the approximated quadratic segments, where p2 of
        // the prior quad is p0 of the next quad. Sized +4 to safely handle the final point
        // insertion and potential SIMD padding.
        std::array<SkPoint, kMaxQuadsFromCubic + 4> fEvenPts;

        // The off-curve control points (p1) of the approximated quadratic segments.
        std::array<SkPoint, kMaxQuadsFromCubic> fOddPts;

        // The starting value of the parabolic integral for each quadratic segment. Maps the start
        // of the curve into uniform arc-length space.
        std::array<float, kMaxQuadsFromCubic> fA0;

        // The delta of the parabolic integral (a2 - a0) for each quadratic. Used to linearly
        // interpolate the integral 'a' across the segment.
        std::array<float, kMaxQuadsFromCubic> fDa;

        // The starting value of the inverse parabolic integral for each quadratic.
        std::array<float, kMaxQuadsFromCubic> fU0;

        // The scale factor for the inverse integral (1.0 / (u2 - u0)). Used to map the evenly
        // spaced parabolic steps back into parametric 't' space.
        std::array<float, kMaxQuadsFromCubic> fUScale;

        // Total integrated curvature/error metric for each quadratic. This dictates how many flat
        // line segments each quad needs to be broken into.
        std::array<float, kMaxQuadsFromCubic> fCurvatureIntegral;

        // The actual number of quadratics the current cubic was split into. Guaranteed to be <=
        // kMaxQuadsFromCubic.
        uint32_t fNumQuads = 0;

        // The final dynamic array where the flattened points of the cubic are gathered before being
        // appended to the main polyline.
        SkTDArray<SkPoint> fFlattenedCubics;
    };

    void processPathsSimd(const SkPath& path, const SkMatrix& ctm, float width, float height,
                          Polyline* polyline);
    void processPathsScalar(const SkPath& path, const SkMatrix& ctm, float width, float height,
                            Polyline* polyline);

    void flattenQuadSimd(const SkPoint pts[3], Polyline* polyline);
    void flattenQuadScalar(const SkPoint pts[3], Polyline* polyline);
    uint32_t flattenCubicScalar(const SkPoint pts[4]);

    void evalCubicsSimd(const SkPoint pts[4], uint32_t numQuads);
    void estimateLinesFromQuadSimd();
    void outputLinesFromQuadSimd(uint32_t quadIdx, float x0, float dx, uint32_t numSegments,
                                 uint32_t startIdx);
    uint32_t flattenCubicSimd(const SkPoint pts[4]);

    CubicFlattenCtx fContext;
    SkAutoConicToQuads fConicToQuad;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_sparse_strips_Flatten_DEFINED
