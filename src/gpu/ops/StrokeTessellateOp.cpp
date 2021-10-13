/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/StrokeTessellateOp.h"

#include "src/core/SkPathPriv.h"
#include "src/gpu/GrAppliedClip.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/tessellate/StrokeFixedCountTessellator.h"
#include "src/gpu/tessellate/StrokeHardwareTessellator.h"
#include "src/gpu/tessellate/shaders/GrTessellationShader.h"

using DynamicStroke = GrStrokeTessellationShader::DynamicStroke;

namespace {

bool can_use_hardware_tessellation(int numVerbs, const GrPipeline& pipeline, const GrCaps& caps) {
    if (!caps.shaderCaps()->tessellationSupport() ||
        !caps.shaderCaps()->infinitySupport() /* The hw tessellation shaders use infinity. */) {
        return false;
    }
    if (pipeline.usesLocalCoords()) {
        // Our back door for HW tessellation shaders isn't currently capable of passing varyings to
        // the fragment shader, so if the processors have varyings, we need to use instanced draws
        // instead.
        return false;
    }
    // Only use hardware tessellation if we're drawing a somewhat large number of verbs. Otherwise
    // we seem to be better off using instanced draws.
    return numVerbs >= caps.minStrokeVerbsForHwTessellation();
}

} // anonymous namespace

namespace skgpu::v1 {

StrokeTessellateOp::StrokeTessellateOp(GrAAType aaType, const SkMatrix& viewMatrix,
                                       const SkPath& path, const SkStrokeRec& stroke,
                                       GrPaint&& paint)
        : GrDrawOp(ClassID())
        , fAAType(aaType)
        , fViewMatrix(viewMatrix)
        , fPathStrokeList(path, stroke, paint.getColor4f())
        , fTotalCombinedVerbCnt(path.countVerbs())
        , fProcessors(std::move(paint)) {
    if (!this->headColor().fitsInBytes()) {
        fShaderFlags |= ShaderFlags::kWideColor;
    }
    SkRect devBounds = path.getBounds();
    if (!this->headStroke().isHairlineStyle()) {
        // Non-hairlines inflate in local path space (pre-transform).
        fInflationRadius = stroke.getInflationRadius();
        devBounds.outset(fInflationRadius, fInflationRadius);
    }
    viewMatrix.mapRect(&devBounds, devBounds);
    if (this->headStroke().isHairlineStyle()) {
        // Hairlines inflate in device space (post-transform).
        fInflationRadius = SkStrokeRec::GetInflationRadius(stroke.getJoin(), stroke.getMiter(),
                                                           stroke.getCap(), 1);
        devBounds.outset(fInflationRadius, fInflationRadius);
    }
    this->setBounds(devBounds, HasAABloat::kNo, IsHairline::kNo);
}

void StrokeTessellateOp::visitProxies(const GrVisitProxyFunc& func) const {
    if (fFillProgram) {
        fFillProgram->visitFPProxies(func);
    } else if (fStencilProgram) {
        fStencilProgram->visitFPProxies(func);
    } else {
        fProcessors.visitProxies(func);
    }
}

GrProcessorSet::Analysis StrokeTessellateOp::finalize(const GrCaps& caps,
                                                      const GrAppliedClip* clip,
                                                      GrClampType clampType) {
    // Make sure the finalize happens before combining. We might change fNeedsStencil here.
    SkASSERT(fPathStrokeList.fNext == nullptr);
    const GrProcessorSet::Analysis& analysis = fProcessors.finalize(
            this->headColor(), GrProcessorAnalysisCoverage::kNone, clip,
            &GrUserStencilSettings::kUnused, caps, clampType, &this->headColor());
    fNeedsStencil = !analysis.unaffectedByDstValue();
    return analysis;
}

GrOp::CombineResult StrokeTessellateOp::onCombineIfPossible(GrOp* grOp, SkArenaAlloc* alloc,
                                                            const GrCaps& caps) {
    SkASSERT(grOp->classID() == this->classID());
    auto* op = static_cast<StrokeTessellateOp*>(grOp);

    // This must be called after finalize(). fNeedsStencil can change in finalize().
    SkASSERT(fProcessors.isFinalized());
    SkASSERT(op->fProcessors.isFinalized());

    if (fNeedsStencil ||
        op->fNeedsStencil ||
        fViewMatrix != op->fViewMatrix ||
        fAAType != op->fAAType ||
        fProcessors != op->fProcessors ||
        this->headStroke().isHairlineStyle() != op->headStroke().isHairlineStyle()) {
        return CombineResult::kCannotCombine;
    }

    auto combinedFlags = fShaderFlags | op->fShaderFlags;
    if (!(combinedFlags & ShaderFlags::kDynamicStroke) &&
        !DynamicStroke::StrokesHaveEqualDynamicState(this->headStroke(), op->headStroke())) {
        // The paths have different stroke properties. We will need to enable dynamic stroke if we
        // still decide to combine them.
        if (this->headStroke().isHairlineStyle()) {
            return CombineResult::kCannotCombine;  // Dynamic hairlines aren't supported.
        }
        combinedFlags |= ShaderFlags::kDynamicStroke;
    }
    if (!(combinedFlags & ShaderFlags::kDynamicColor) && this->headColor() != op->headColor()) {
        // The paths have different colors. We will need to enable dynamic color if we still decide
        // to combine them.
        combinedFlags |= ShaderFlags::kDynamicColor;
    }

    // Don't actually enable new dynamic state on ops that already have lots of verbs.
    constexpr static GrTFlagsMask<ShaderFlags> kDynamicStatesMask(ShaderFlags::kDynamicStroke |
                                                                  ShaderFlags::kDynamicColor);
    ShaderFlags neededDynamicStates = combinedFlags & kDynamicStatesMask;
    if (neededDynamicStates != ShaderFlags::kNone) {
        if (!this->shouldUseDynamicStates(neededDynamicStates) ||
            !op->shouldUseDynamicStates(neededDynamicStates)) {
            return CombineResult::kCannotCombine;
        }
    }

    fShaderFlags = combinedFlags;

    // Concat the op's PathStrokeList. Since the head element is allocated inside the op, we need to
    // copy it.
    auto* headCopy = alloc->make<PathStrokeList>(std::move(op->fPathStrokeList));
    *fPathStrokeTail = headCopy;
    fPathStrokeTail = (op->fPathStrokeTail == &op->fPathStrokeList.fNext) ? &headCopy->fNext
                                                                          : op->fPathStrokeTail;

    fInflationRadius = std::max(fInflationRadius, op->fInflationRadius);
    fTotalCombinedVerbCnt += op->fTotalCombinedVerbCnt;
    return CombineResult::kMerged;
}

// Marks every stencil value as "1".
constexpr static GrUserStencilSettings kMarkStencil(
    GrUserStencilSettings::StaticInit<
        0x0001,
        GrUserStencilTest::kLessIfInClip,  // Match kTestAndResetStencil.
        0x0000,  // Always fail.
        GrUserStencilOp::kZero,
        GrUserStencilOp::kReplace,
        0xffff>());

// Passes if the stencil value is nonzero. Also resets the stencil value to zero on pass. This is
// formulated to match kMarkStencil everywhere except the ref and compare mask. This will allow us
// to use the same pipeline for both stencil and fill if dynamic stencil state is supported.
constexpr static GrUserStencilSettings kTestAndResetStencil(
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kLessIfInClip,  // i.e., "not equal to zero, if in clip".
        0x0001,
        GrUserStencilOp::kZero,
        GrUserStencilOp::kReplace,
        0xffff>());

void StrokeTessellateOp::prePrepareTessellator(GrTessellationShader::ProgramArgs&& args,
                                               GrAppliedClip&& clip) {
    SkASSERT(!fTessellator);
    SkASSERT(!fFillProgram);
    SkASSERT(!fStencilProgram);
    // GrOp::setClippedBounds() should have been called by now.
    SkASSERT(SkRect::MakeIWH(args.fWriteView.width(),
                             args.fWriteView.height()).contains(this->bounds()));

    const GrCaps& caps = *args.fCaps;
    SkArenaAlloc* arena = args.fArena;

    std::array<float, 2> matrixMinMaxScales;
    if (!fViewMatrix.getMinMaxScales(matrixMinMaxScales.data())) {
        matrixMinMaxScales.fill(1);
    }

    float devInflationRadius = fInflationRadius;
    if (!this->headStroke().isHairlineStyle()) {
        devInflationRadius *= matrixMinMaxScales[1];
    }
    SkRect strokeCullBounds = this->bounds().makeOutset(devInflationRadius, devInflationRadius);

    auto* pipeline = GrTessellationShader::MakePipeline(args, fAAType, std::move(clip),
                                                        std::move(fProcessors));

    if (can_use_hardware_tessellation(fTotalCombinedVerbCnt, *pipeline, caps)) {
        // Only use hardware tessellation if we're drawing a somewhat large number of verbs.
        // Otherwise we seem to be better off using instanced draws.
        fTessellator = arena->make<skgpu::tess::StrokeHardwareTessellator>(*caps.shaderCaps(),
                                                                           fShaderFlags,
                                                                           fViewMatrix,
                                                                           &fPathStrokeList,
                                                                           matrixMinMaxScales,
                                                                           strokeCullBounds);
    } else {
        fTessellator = arena->make<skgpu::tess::StrokeFixedCountTessellator>(*caps.shaderCaps(),
                                                                             fShaderFlags,
                                                                             fViewMatrix,
                                                                             &fPathStrokeList,
                                                                             matrixMinMaxScales,
                                                                             strokeCullBounds);
    }

    auto fillStencil = &GrUserStencilSettings::kUnused;
    if (fNeedsStencil) {
        fStencilProgram = GrTessellationShader::MakeProgram(args, fTessellator->shader(), pipeline,
                                                            &kMarkStencil);
        fillStencil = &kTestAndResetStencil;
        args.fXferBarrierFlags = GrXferBarrierFlags::kNone;
    }

    fFillProgram = GrTessellationShader::MakeProgram(args, fTessellator->shader(), pipeline,
                                                     fillStencil);
}

void StrokeTessellateOp::onPrePrepare(GrRecordingContext* context,
                                      const GrSurfaceProxyView& writeView, GrAppliedClip* clip,
                                      const GrDstProxyView& dstProxyView,
                                      GrXferBarrierFlags renderPassXferBarriers, GrLoadOp
                                      colorLoadOp) {
    // DMSAA is not supported on DDL.
    bool usesMSAASurface = writeView.asRenderTargetProxy()->numSamples() > 1;
    this->prePrepareTessellator({context->priv().recordTimeAllocator(), writeView, usesMSAASurface,
                                &dstProxyView, renderPassXferBarriers, colorLoadOp,
                                context->priv().caps()},
                                (clip) ? std::move(*clip) : GrAppliedClip::Disabled());
    if (fStencilProgram) {
        context->priv().recordProgramInfo(fStencilProgram);
    }
    if (fFillProgram) {
        context->priv().recordProgramInfo(fFillProgram);
    }
}

void StrokeTessellateOp::onPrepare(GrOpFlushState* flushState) {
    if (!fTessellator) {
        this->prePrepareTessellator({flushState->allocator(), flushState->writeView(),
                                    flushState->usesMSAASurface(), &flushState->dstProxyView(),
                                    flushState->renderPassBarriers(), flushState->colorLoadOp(),
                                    &flushState->caps()}, flushState->detachAppliedClip());
    }
    SkASSERT(fTessellator);
    fTessellator->prepare(flushState, fTotalCombinedVerbCnt);
}

void StrokeTessellateOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
    if (fStencilProgram) {
        flushState->bindPipelineAndScissorClip(*fStencilProgram, chainBounds);
        flushState->bindTextures(fStencilProgram->geomProc(), nullptr, fStencilProgram->pipeline());
        fTessellator->draw(flushState);
    }
    if (fFillProgram) {
        flushState->bindPipelineAndScissorClip(*fFillProgram, chainBounds);
        flushState->bindTextures(fFillProgram->geomProc(), nullptr, fFillProgram->pipeline());
        fTessellator->draw(flushState);
    }
}

} // namespace skgpu::v1
