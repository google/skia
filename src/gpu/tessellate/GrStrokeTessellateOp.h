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
    GrStrokeTessellateOp(GrAAType, const SkMatrix&, const SkStrokeRec&, const SkPath&, GrPaint&&);

    const char* name() const override { return "GrStrokeTessellateOp"; }
    void visitProxies(const VisitProxyFunc& fn) const override { fProcessors.visitProxies(fn); }
    FixedFunctionFlags fixedFunctionFlags() const override;
    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*,
                                      bool hasMixedSampledCoverage, GrClampType) override;
    CombineResult onCombineIfPossible(GrOp*, GrRecordingContext::Arenas*, const GrCaps&) override;
    void onPrePrepare(GrRecordingContext*, const GrSurfaceProxyView*, GrAppliedClip*,
                      const GrXferProcessor::DstProxyView&, GrXferBarrierFlags) override;
    void onPrepare(GrOpFlushState* state) override;

    void prePrepareColorProgram(SkArenaAlloc*, const GrSurfaceProxyView*, GrAppliedClip&&, const
                                GrXferProcessor::DstProxyView&, GrXferBarrierFlags, const GrCaps&);

    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;

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

    GrSTArenaList<SkPath> fPaths;
    int fTotalCombinedVerbCnt;

    const GrProgramInfo* fColorProgram = nullptr;

    // S=1 because we will almost always fit everything into one single chunk.
    SkSTArray<1, GrStrokePatchBuilder::PatchChunk> fPatchChunks;

    friend class GrOp;  // For ctor.
};

#endif
