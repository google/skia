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
#include "src/gpu/tessellate/GrStrokeTessellateShader.h"
#include <array>

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
    void visitProxies(const VisitProxyFunc& fn) const override;
    FixedFunctionFlags fixedFunctionFlags() const override;
    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*,
                                      bool hasMixedSampledCoverage, GrClampType) override;
    CombineResult onCombineIfPossible(GrOp*, SkArenaAlloc*, const GrCaps&) override;

    void prePreparePrograms(GrStrokeTessellateShader::Mode, SkArenaAlloc*,
                            const GrSurfaceProxyView&, GrAppliedClip&&,
                            const GrXferProcessor::DstProxyView&, GrXferBarrierFlags,
                            GrLoadOp colorLoadOp, const GrCaps&);

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

    // Returns the equivalent tolerances in (pre-viewMatrix) local path space that the tessellator
    // will use when rendering this stroke.
    GrStrokeTessellateShader::Tolerances preTransformTolerances() const {
        std::array<float,2> matrixScales;
        if (!fViewMatrix.getMinMaxScales(matrixScales.data())) {
            matrixScales.fill(1);
        }
        auto [matrixMinScale, matrixMaxScale] = matrixScales;
        float localStrokeWidth = fStroke.getWidth();
        if (fStroke.isHairlineStyle()) {
            // If the stroke is hairline then the tessellator will operate in post-transform space
            // instead. But for the sake of CPU methods that need to conservatively approximate the
            // number of segments to emit, we use localStrokeWidth ~= 1/matrixMinScale.
            float approxScale = matrixMinScale;
            // If the matrix has strong skew, don't let the scale shoot off to infinity. (This does
            // not affect the tessellator; only the CPU methods that approximate the number of
            // segments to emit.)
            approxScale = std::max(matrixMinScale, matrixMaxScale * .25f);
            localStrokeWidth = 1/approxScale;
        }
        return GrStrokeTessellateShader::Tolerances(matrixMaxScale, localStrokeWidth);
    }

    const GrAAType fAAType;
    const SkMatrix fViewMatrix;
    const SkStrokeRec fStroke;
    SkPMColor4f fColor;
    bool fNeedsStencil = false;
    GrProcessorSet fProcessors;

    GrSTArenaList<SkPath> fPathList;
    int fTotalCombinedVerbCnt = 0;
    int fTotalConicWeightCnt = 0;

    const GrProgramInfo* fStencilProgram = nullptr;
    const GrProgramInfo* fFillProgram = nullptr;
};

#endif
