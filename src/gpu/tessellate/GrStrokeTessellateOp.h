/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStrokeTessellateOp_DEFINED
#define GrStrokeTessellateOp_DEFINED

#include "include/core/SkStrokeRec.h"
#include "src/gpu/ops/GrMeshDrawOp.h"
#include "src/gpu/tessellate/GrPathShader.h"
#include "src/gpu/tessellate/GrStrokeTessellateShader.h"

class GrRecordingContext;

// Prepares GPU data for, and then draws a stroke's tessellated geometry.
class GrStrokeTessellator {
public:
    using ShaderFlags = GrStrokeTessellateShader::ShaderFlags;

    struct PathStrokeList {
        PathStrokeList(const SkPath& path, const SkStrokeRec& stroke, const SkPMColor4f& color)
                : fPath(path), fStroke(stroke), fColor(color) {}
        SkPath fPath;
        SkStrokeRec fStroke;
        SkPMColor4f fColor;
        PathStrokeList* fNext = nullptr;
    };

    GrStrokeTessellator(ShaderFlags shaderFlags, PathStrokeList* pathStrokeList)
            : fShaderFlags(shaderFlags), fPathStrokeList(pathStrokeList) {}

    // Called before draw(). Prepares GPU buffers containing the geometry to tessellate.
    virtual void prepare(GrMeshDrawOp::Target*, const SkMatrix&) = 0;

    // Issues draw calls for the tessellated stroke. The caller is responsible for binding its
    // desired pipeline ahead of time.
    virtual void draw(GrOpFlushState*) const = 0;

    virtual ~GrStrokeTessellator() {}

protected:
    const ShaderFlags fShaderFlags;
    PathStrokeList* fPathStrokeList;
};

// Renders strokes by linearizing them into sorted "parametric" and "radial" edges. See
// GrStrokeTessellateShader.
class GrStrokeTessellateOp : public GrDrawOp {
public:
    GrStrokeTessellateOp(GrAAType, const SkMatrix&, const SkPath&, const SkStrokeRec&, GrPaint&&);

private:
    using ShaderFlags = GrStrokeTessellateShader::ShaderFlags;
    using PathStrokeList = GrStrokeTessellator::PathStrokeList;
    DEFINE_OP_CLASS_ID

    SkStrokeRec& headStroke() { return fPathStrokeList.fStroke; }
    SkPMColor4f& headColor() { return fPathStrokeList.fColor; }
    GrStrokeTessellateOp* nextInChain() const {
        return static_cast<GrStrokeTessellateOp*>(this->GrDrawOp::nextInChain());
    }

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

    bool canUseHardwareTessellation(const GrCaps& caps) {
        SkASSERT(!fStencilProgram && !fFillProgram);  // Ensure we haven't std::moved fProcessors.
        // Our back door for HW tessellation shaders isn't currently capable of passing varyings to
        // the fragment shader, so if the processors have varyings we need to use indirect draws.
        return caps.shaderCaps()->tessellationSupport() && !fProcessors.usesVaryingCoords();
    }

    const char* name() const override { return "GrStrokeTessellateOp"; }
    void visitProxies(const VisitProxyFunc& fn) const override;
    FixedFunctionFlags fixedFunctionFlags() const override;
    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*,
                                      bool hasMixedSampledCoverage, GrClampType) override;
    CombineResult onCombineIfPossible(GrOp*, SkArenaAlloc*, const GrCaps&) override;

    // Creates the tessellator and the stencil/fill program(s) we will use with it.
    void prePrepareTessellator(GrPathShader::ProgramArgs&&, GrAppliedClip&&);

    void onPrePrepare(GrRecordingContext*, const GrSurfaceProxyView&, GrAppliedClip*,
                      const GrXferProcessor::DstProxyView&, GrXferBarrierFlags,
                      GrLoadOp colorLoadOp) override;

    void onPrepare(GrOpFlushState*) override;

    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;

    const GrAAType fAAType;
    const SkMatrix fViewMatrix;
    ShaderFlags fShaderFlags = ShaderFlags::kNone;
    PathStrokeList fPathStrokeList;
    PathStrokeList** fPathStrokeTail = &fPathStrokeList.fNext;
    int fTotalCombinedVerbCnt = 0;
    GrProcessorSet fProcessors;
    bool fNeedsStencil = false;

    GrStrokeTessellator* fTessellator = nullptr;
    const GrProgramInfo* fStencilProgram = nullptr;  // Only used if the stroke has transparency.
    const GrProgramInfo* fFillProgram = nullptr;
};

#endif
