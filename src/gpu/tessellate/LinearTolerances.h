/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_tessellate_LinearTolerances_DEFINED
#define skgpu_tessellate_LinearTolerances_DEFINED

#include "include/core/SkScalar.h"
#include "include/private/base/SkAssert.h"
#include "src/gpu/tessellate/Tessellation.h"
#include "src/gpu/tessellate/WangsFormula.h"

#include <algorithm>

namespace skgpu::tess {

/**
 * LinearTolerances stores state to approximate the final device-space transform applied
 * to curves, and uses that to calculate segmentation levels for both the parametric curves and
 * radial components (when stroking, where you have to represent the offset of a curve).
 * These tolerances determine the worst-case number of parametric and radial segments required to
 * accurately linearize curves.
 * - segments = a linear subsection on the curve, either defined as parametric (linear in t) or
 *   radial (linear in curve's internal rotation).
 * - edges = orthogonal geometry to segments, used in stroking to offset from the central curve by
 *   half the stroke width, or to construct the join geometry.
 *
 * The tolerance values and decisions are estimated in the local path space, although PatchWriter
 * uses a 2x2 vector transform that approximates the scale/skew (as-best-as-possible) of the full
 * local-to-device transform applied in the vertex shader.
 *
 * The properties tracked in LinearTolerances can be used to compute the final segmentation factor
 * for filled paths (the resolve level) or stroked paths (the number of edges).
 */
class LinearTolerances {
public:
    float numParametricSegments_p4() const { return fNumParametricSegments_p4; }
    float numRadialSegmentsPerRadian() const { return fNumRadialSegmentsPerRadian; }
    int   numEdgesInJoins() const { return fEdgesInJoins; }

    // Fast log2 of minimum required # of segments per tracked Wang's formula calculations.
    int requiredResolveLevel() const {
        // log16(n^4) == log2(n)
        return wangs_formula::nextlog16(fNumParametricSegments_p4);
    }

    int requiredStrokeEdges() const {
        // The maximum rotation we can have in a stroke is 180 degrees (SK_ScalarPI radians).
        int maxRadialSegmentsInStroke =
                std::max(SkScalarCeilToInt(fNumRadialSegmentsPerRadian * SK_ScalarPI), 1);

        int maxParametricSegmentsInStroke =
                SkScalarCeilToInt(wangs_formula::root4(fNumParametricSegments_p4));
        SkASSERT(maxParametricSegmentsInStroke >= 1);

        // Now calculate the maximum number of edges we will need in the stroke portion of the
        // instance. The first and last edges in a stroke are shared by both the parametric and
        // radial sets of edges, so the total number of edges is:
        //
        //   numCombinedEdges = numParametricEdges + numRadialEdges - 2
        //
        // It's important to differentiate between the number of edges and segments in a strip:
        //
        //   numSegments = numEdges - 1
        //
        // So the total number of combined edges in the stroke is:
        //
        //   numEdgesInStroke = numParametricSegments + 1 + numRadialSegments + 1 - 2
        //                    = numParametricSegments + numRadialSegments
        //
        int maxEdgesInStroke = maxRadialSegmentsInStroke + maxParametricSegmentsInStroke;

        // Each triangle strip has two sections: It starts with a join then transitions to a
        // stroke. The number of edges in an instance is the sum of edges from the join and
        // stroke sections both.
        // NOTE: The final join edge and the first stroke edge are co-located, however we still
        // need to emit both because the join's edge is half-width and the stroke is full-width.
        return fEdgesInJoins + maxEdgesInStroke;
    }

    void setParametricSegments(float n4) {
        SkASSERT(n4 >= 0.f);
        fNumParametricSegments_p4 = n4;
    }

    void setStroke(const StrokeParams& strokeParams, float maxScale) {
        float approxDeviceStrokeRadius;
        if (strokeParams.fRadius == 0.f) {
            // Hairlines are always 1 px wide
            approxDeviceStrokeRadius = 0.5f;
        } else {
            // Approximate max scale * local stroke width / 2
            approxDeviceStrokeRadius = strokeParams.fRadius * maxScale;
        }

        fNumRadialSegmentsPerRadian = CalcNumRadialSegmentsPerRadian(approxDeviceStrokeRadius);

        fEdgesInJoins = NumFixedEdgesInJoin(strokeParams);
        if (strokeParams.fJoinType < 0.f && fNumRadialSegmentsPerRadian > 0.f) {
            // For round joins we need to count the radial edges on our own. Account for a
            // worst-case join of 180 degrees (SK_ScalarPI radians).
            fEdgesInJoins += SkScalarCeilToInt(fNumRadialSegmentsPerRadian * SK_ScalarPI) - 1;
        }
    }

    void accumulate(const LinearTolerances& tolerances) {
        if (tolerances.fNumParametricSegments_p4 > fNumParametricSegments_p4) {
            fNumParametricSegments_p4 = tolerances.fNumParametricSegments_p4;
        }
        if (tolerances.fNumRadialSegmentsPerRadian > fNumRadialSegmentsPerRadian) {
            fNumRadialSegmentsPerRadian = tolerances.fNumRadialSegmentsPerRadian;
        }
        if (tolerances.fEdgesInJoins > fEdgesInJoins) {
            fEdgesInJoins = tolerances.fEdgesInJoins;
        }
    }

private:
    // Used for both fills and strokes, always at least one parametric segment
    float fNumParametricSegments_p4 = 1.f;
    // Used for strokes, adding additional segments along the curve to account for its rotation
    // TODO: Currently we assume the worst case 180 degree rotation for any curve, but tracking
    // max(radialSegments * patch curvature) would be tighter. This would require computing
    // rotation per patch, which could be approximated by tracking min of the tangent dot
    // products, but then we'd be left with the slightly less accurate
    // "max(radialSegments) * acos(min(tan dot product))". It is also unknown if requesting
    // tighter bounds pays off with less GPU work for more CPU work
    float fNumRadialSegmentsPerRadian = 0.f;
    // Used for strokes, tracking the number of additional vertices required to handle joins
    // based on the join type and stroke width.
    // TODO: For round joins, we could also track the rotation angle of the join, instead of
    // assuming 180 degrees. PatchWriter has all necessary control points to do so, but runs
    // into similar trade offs between CPU vs GPU work, and accuracy vs. reducing calls to acos.
    int   fEdgesInJoins = 0;
};

}  // namespace skgpu::tess

#endif // skgpu_tessellate_LinearTolerances_DEFINED
