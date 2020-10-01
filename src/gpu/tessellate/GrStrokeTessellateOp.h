/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStrokeTessellateOp_DEFINED
#define GrStrokeTessellateOp_DEFINED

#include "include/core/SkStrokeRec.h"
#include "src/gpu/GrTArenaList.h"
#include "src/gpu/ops/GrDrawOp.h"
#include "src/gpu/tessellate/GrStrokePatchBuilder.h"

// Renders opaque, constant-color strokes by decomposing them into standalone tessellation patches.
// Each patch is either a "cubic" (single stroked bezier curve with butt caps) or a "join". Requires
// MSAA if antialiasing is desired.
class GrStrokeTessellateOp : public GrDrawOp {
public:
    DEFINE_OP_CLASS_ID

    // Patches can overlap, so until a stencil technique is implemented, the provided paint must be
    // a constant blended color.
    static std::unique_ptr<GrStrokeTessellateOp> Make(GrRecordingContext::Arenas, GrAAType,
                                                      const SkMatrix&, const SkStrokeRec&,
                                                      const SkPath&, GrPaint&&);

private:
    GrStrokeTessellateOp(SkArenaAlloc* recordTimeAllocator, GrAAType, const SkMatrix&,
                         const SkStrokeRec&, const SkPath&, GrPaint&&);

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
    const float fMatrixScale;
    const SkStrokeRec fStroke;
    SkPMColor4f fColor;
    GrProcessorSet fProcessors;

    GrTArenaList<SkPath> fPathList;
    int fTotalCombinedVerbCnt;

    const GrProgramInfo* fColorProgram = nullptr;

    // S=1 because we will almost always fit everything into one single chunk.
    SkSTArray<1, GrStrokePatchBuilder::PatchChunk> fPatchChunks;

    friend class GrOpMemoryPool;  // For ctor.
};

#endif
