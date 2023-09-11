/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef StrokeTessellateOp_DEFINED
#define StrokeTessellateOp_DEFINED

#include "include/core/SkStrokeRec.h"
#include "src/gpu/ganesh/ops/GrDrawOp.h"
#include "src/gpu/ganesh/tessellate/GrTessellationShader.h"
#include "src/gpu/ganesh/tessellate/StrokeTessellator.h"

class GrRecordingContext;
class GrStrokeTessellationShader;

namespace skgpu::ganesh {

// Renders strokes by linearizing them into sorted "parametric" and "radial" edges. See
// GrStrokeTessellationShader.
class StrokeTessellateOp final : public GrDrawOp {
public:
    StrokeTessellateOp(GrAAType, const SkMatrix&, const SkPath&, const SkStrokeRec&, GrPaint&&);

private:
    using PatchAttribs = StrokeTessellator::PatchAttribs;
    using PathStrokeList = StrokeTessellator::PathStrokeList;

    DEFINE_OP_CLASS_ID

    SkStrokeRec& headStroke() { return fPathStrokeList.fStroke; }
    SkPMColor4f& headColor() { return fPathStrokeList.fColor; }

    // Returns whether it is a good tradeoff to use the dynamic states flagged in the given
    // bitfield. Dynamic states improve batching, but if they aren't already enabled, they come at
    // the cost of having to write out more data with each patch or instance.
    bool shouldUseDynamicStates(PatchAttribs neededDynamicStates) const {
        // Use the dynamic states if either (1) they are all already enabled anyway, or (2) we don't
        // have many verbs.
        constexpr static int kMaxVerbsToEnableDynamicState = 50;
        bool anyStateDisabled = (bool)(~fPatchAttribs & neededDynamicStates);
        bool allStatesEnabled = !anyStateDisabled;
        return allStatesEnabled || (fTotalCombinedVerbCnt <= kMaxVerbsToEnableDynamicState);
    }

    const char* name() const override { return "StrokeTessellateOp"; }
    void visitProxies(const GrVisitProxyFunc&) const override;
    bool usesMSAA() const override { return fAAType == GrAAType::kMSAA; }
    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*, GrClampType) override;
    bool usesStencil() const override {
        // This must be called after finalize(). fNeedsStencil can change in finalize().
        SkASSERT(fProcessors.isFinalized());
        return fNeedsStencil;
    }
    CombineResult onCombineIfPossible(GrOp*, SkArenaAlloc*, const GrCaps&) override;

    // Creates the tessellator and the stencil/fill program(s) we will use with it.
    void prePrepareTessellator(GrTessellationShader::ProgramArgs&&, GrAppliedClip&&);

    void onPrePrepare(GrRecordingContext*, const GrSurfaceProxyView&, GrAppliedClip*,
                      const GrDstProxyView&, GrXferBarrierFlags, GrLoadOp colorLoadOp) override;

    void onPrepare(GrOpFlushState*) override;

    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;

    const GrAAType fAAType;
    const SkMatrix fViewMatrix;
    PatchAttribs fPatchAttribs = PatchAttribs::kNone;
    PathStrokeList fPathStrokeList;
    PathStrokeList** fPathStrokeTail = &fPathStrokeList.fNext;
    int fTotalCombinedVerbCnt = 0;
    GrProcessorSet fProcessors;
    bool fNeedsStencil;

    StrokeTessellator* fTessellator = nullptr;
    GrStrokeTessellationShader* fTessellationShader;
    const GrProgramInfo* fStencilProgram = nullptr;  // Only used if the stroke has transparency.
    const GrProgramInfo* fFillProgram = nullptr;
};

}  // namespace skgpu::ganesh

#endif // StrokeTessellateOp_DEFINED
