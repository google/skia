/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStrokeTessellateOp_DEFINED
#define GrStrokeTessellateOp_DEFINED

#include "include/core/SkStrokeRec.h"
#include "src/gpu/GrSTArenaList.h"
#include "src/gpu/ops/GrDrawOp.h"
#include "src/gpu/tessellate/GrStrokePatchBuilder.h"

// Renders opaque, constant-color strokes by decomposing them into standalone tessellation patches.
// Each patch is either a "cubic" (single stroked bezier curve with butt caps) or a "join". Requires
// MSAA if antialiasing is desired.
class GrStrokeTessellateOp : public GrDrawOp {
public:
    DEFINE_OP_CLASS_ID

private:
    // The provided matrix must be a similarity matrix for the time being. This is so we can
    // bootstrap this Op on top of GrStrokeGeometry with minimal modifications.
    //
    // Patches can overlap, so until a stencil technique is implemented, the provided paint must be
    // a constant blended color.
    GrStrokeTessellateOp(GrAAType, const SkMatrix&, const SkPath&, const SkStrokeRec&, GrPaint&&);

    const char* name() const override { return "GrStrokeTessellateOp"; }
    void visitProxies(const VisitProxyFunc& fn) const override { fProcessors.visitProxies(fn); }
    FixedFunctionFlags fixedFunctionFlags() const override;
    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*,
                                      bool hasMixedSampledCoverage, GrClampType) override;
    CombineResult onCombineIfPossible(GrOp*, GrRecordingContext::Arenas*, const GrCaps&) override;
    void onPrePrepare(GrRecordingContext*, const GrSurfaceProxyView*, GrAppliedClip*,
                      const GrXferProcessor::DstProxyView&) override;
    void onPrepare(GrOpFlushState* state) override;
    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;

    struct PathStroke {
        PathStroke(const SkPath& path, const SkStrokeRec& stroke) : fPath(path), fStroke(stroke) {}
        SkPath fPath;
        SkStrokeRec fStroke;
    };

    GrSTArenaList<PathStroke> fPathStrokes;
    int fTotalCombinedVerbCnt;

    const GrAAType fAAType;
    float fMatrixScale;  // The matrix scale is applied to control points before tessellation.
    SkMatrix fSkewMatrix;  // The skew matrix is applied to the post-tessellation triangles.
    float fMiterLimitOrZero = 0;  // Zero if there is not a stroke with a miter join type.
    SkPMColor4f fColor;
    GrProcessorSet fProcessors;

    // S=1 because we will almost always fit everything into one single chunk.
    SkSTArray<1, GrStrokePatchBuilder::VertexChunk> fVertexChunks;

    friend class GrOpMemoryPool;  // For ctor.
};

#endif
