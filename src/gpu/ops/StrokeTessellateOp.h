/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef StrokeTessellateOp_DEFINED
#define StrokeTessellateOp_DEFINED

#include "include/core/SkStrokeRec.h"
#include "src/gpu/ops/GrDrawOp.h"
#include "src/gpu/tessellate/StrokeTessellator.h"
#include "src/gpu/tessellate/shaders/GrTessellationShader.h"

class GrRecordingContext;

namespace skgpu::v1 {

// Renders strokes by linearizing them into sorted "parametric" and "radial" edges. See
// GrStrokeTessellationShader.
class StrokeTessellateOp final : public GrDrawOp {
public:
    StrokeTessellateOp(GrAAType, const SkMatrix&, const SkPath&, const SkStrokeRec&, GrPaint&&);

private:
    using ShaderFlags = GrStrokeTessellationShader::ShaderFlags;
    using PathStrokeList = StrokeTessellator::PathStrokeList;
    DEFINE_OP_CLASS_ID

    SkStrokeRec& headStroke() { return fPathStrokeList.fStroke; }
    SkPMColor4f& headColor() { return fPathStrokeList.fColor; }

    // Returns whether it is a good tradeoff to use the dynamic states flagged in the given
    // bitfield. Dynamic states improve batching, but if they aren't already enabled, they come at
    // the cost of having to write out more data with each patch or instance.
    bool shouldUseDynamicStates(ShaderFlags neededDynamicStates) const {
        // Use the dynamic states if either (1) they are all already enabled anyway, or (2) we don't
        // have many verbs.
        constexpr static int kMaxVerbsToEnableDynamicState = 50;
        bool anyStateDisabled = (bool)(~fShaderFlags & neededDynamicStates);
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
    ShaderFlags fShaderFlags = ShaderFlags::kNone;
    PathStrokeList fPathStrokeList;
    PathStrokeList** fPathStrokeTail = &fPathStrokeList.fNext;
    float fInflationRadius = 0;
    int fTotalCombinedVerbCnt = 0;
    GrProcessorSet fProcessors;
    bool fNeedsStencil;

    StrokeTessellator* fTessellator = nullptr;
    const GrProgramInfo* fStencilProgram = nullptr;  // Only used if the stroke has transparency.
    const GrProgramInfo* fFillProgram = nullptr;
};

} // namespace skgpu::v1

#endif // StrokeTessellateOp_DEFINED
