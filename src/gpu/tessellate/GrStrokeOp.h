/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStrokeOp_DEFINED
#define GrStrokeOp_DEFINED

#include "include/core/SkStrokeRec.h"
#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/GrSTArenaList.h"
#include "src/gpu/ops/GrDrawOp.h"

class GrStrokeTessellateShader;

// Base class for ops that render opaque, constant-color strokes by linearizing them into sorted
// "parametric" and "radial" edges. See GrStrokeTessellateShader.
class GrStrokeOp : public GrDrawOp {
protected:
    // The provided matrix must be a similarity matrix for the time being. This is so we can
    // bootstrap this Op on top of GrStrokeGeometry with minimal modifications.
    //
    // Patches can overlap, so until a stencil technique is implemented, the provided paint must be
    // a constant blended color.
    GrStrokeOp(uint32_t classID, GrAAType, const SkMatrix&, const SkStrokeRec&, const SkPath&,
               GrPaint&&);

    const char* name() const override { return "GrStrokeTessellateOp"; }
    void visitProxies(const VisitProxyFunc& fn) const override { fProcessors.visitProxies(fn); }
    FixedFunctionFlags fixedFunctionFlags() const override;
    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*,
                                      bool hasMixedSampledCoverage, GrClampType) override;
    CombineResult onCombineIfPossible(GrOp*, SkArenaAlloc*, const GrCaps&) override;

    void prePrepareColorProgram(SkArenaAlloc* arena, GrStrokeTessellateShader*,
                                const GrSurfaceProxyView*, GrAppliedClip&&, const
                                GrXferProcessor::DstProxyView&, GrXferBarrierFlags, const GrCaps&);

    static float NumCombinedSegments(float numParametricSegments, float numRadialSegments) {
        // The first and last edges are shared by both the parametric and radial sets of edges, so
        // the total number of edges is:
        //
        //   numCombinedEdges = numParametricEdges + numRadialEdges - 2
        //
        // It's also important to differentiate between the number of edges and segments in a strip:
        //
        //   numCombinedSegments = numCombinedEdges - 1
        //
        // So the total number of segments in the combined strip is:
        //
        //   numCombinedSegments = numParametricEdges + numRadialEdges - 2 - 1
        //                       = numParametricSegments + 1 + numRadialSegments + 1 - 2 - 1
        //                       = numParametricSegments + numRadialSegments - 1
        //
        return numParametricSegments + numRadialSegments - 1;
    }

    static float NumParametricSegments(float numCombinedSegments, float numRadialSegments) {
        // numCombinedSegments = numParametricSegments + numRadialSegments - 1.
        // (See num_combined_segments()).
        return std::max(numCombinedSegments + 1 - numRadialSegments, 0.f);
    }

    const GrAAType fAAType;
    const SkMatrix fViewMatrix;
    const SkStrokeRec fStroke;
    // Controls the number of parametric segments the tessellator adds for each curve. The
    // tessellator will add enough parametric segments so that the center of each one falls within
    // 1/parametricIntolerance local path units from the true curve.
    const float fParametricIntolerance;
    // Controls the number of radial segments the tessellator adds for each curve. The tessellator
    // will add this number of radial segments for each radian of rotation, in order to guarantee
    // smoothness.
    const float fNumRadialSegmentsPerRadian;
    SkPMColor4f fColor;
    GrProcessorSet fProcessors;

    GrSTArenaList<SkPath> fPathList;
    int fTotalCombinedVerbCnt;

    const GrProgramInfo* fColorProgram = nullptr;
};

#endif
