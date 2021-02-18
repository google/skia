/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrStrokeTessellateOp.h"

#include "src/core/SkPathPriv.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/tessellate/GrFillPathShader.h"
#include "src/gpu/tessellate/GrStencilPathShader.h"
#include "src/gpu/tessellate/GrStrokeHardwareTessellator.h"
#include "src/gpu/tessellate/GrStrokeIndirectTessellator.h"

GrStrokeTessellateOp::GrStrokeTessellateOp(GrAAType aaType, const SkMatrix& viewMatrix,
                                           const SkPath& path, const SkStrokeRec& stroke,
                                           GrPaint&& paint)
        : GrDrawOp(ClassID())
        , fAAType(aaType)
        , fViewMatrix(viewMatrix)
        , fColor(paint.getColor4f())
        , fProcessors(std::move(paint))
        , fPathStrokeList(path, stroke)
        , fTotalCombinedVerbCnt(path.countVerbs()) {
    if (SkPathPriv::ConicWeightCnt(path) != 0) {
        fShaderFlags |= ShaderFlags::kHasConics;
    }
    SkRect devBounds = path.getBounds();
    float inflationRadius = stroke.getInflationRadius();
    devBounds.outset(inflationRadius, inflationRadius);
    viewMatrix.mapRect(&devBounds, devBounds);
    this->setBounds(devBounds, HasAABloat(GrAAType::kCoverage == fAAType), IsHairline::kNo);
}

void GrStrokeTessellateOp::visitProxies(const VisitProxyFunc& fn) const {
    if (fFillProgram) {
        fFillProgram->visitFPProxies(fn);
    } else if (fStencilProgram) {
        fStencilProgram->visitFPProxies(fn);
    } else {
        fProcessors.visitProxies(fn);
    }
}

GrDrawOp::FixedFunctionFlags GrStrokeTessellateOp::fixedFunctionFlags() const {
    // We might not actually end up needing stencil, but won't know for sure until finalize().
    // Request it just in case we do end up needing it.
    auto flags = FixedFunctionFlags::kUsesStencil;
    if (GrAAType::kNone != fAAType) {
        flags |= FixedFunctionFlags::kUsesHWAA;
    }
    return flags;
}

GrProcessorSet::Analysis GrStrokeTessellateOp::finalize(const GrCaps& caps,
                                                        const GrAppliedClip* clip,
                                                        bool hasMixedSampledCoverage,
                                                        GrClampType clampType) {
    // Make sure the finalize happens before combining. We might change fNeedsStencil here.
    SkASSERT(fPathStrokeList.begin().fCurr->fNext == nullptr);
    SkASSERT(fAAType != GrAAType::kCoverage || hasMixedSampledCoverage);
    const GrProcessorSet::Analysis& analysis = fProcessors.finalize(
            fColor, GrProcessorAnalysisCoverage::kNone, clip, &GrUserStencilSettings::kUnused,
            hasMixedSampledCoverage, caps, clampType, &fColor);
    fNeedsStencil = !analysis.unaffectedByDstValue();
    return analysis;
}

GrOp::CombineResult GrStrokeTessellateOp::onCombineIfPossible(GrOp* grOp, SkArenaAlloc* alloc,
                                                              const GrCaps&) {
    using DynamicStroke = GrStrokeTessellateShader::DynamicStroke;
    SkASSERT(grOp->classID() == this->classID());
    auto* op = static_cast<GrStrokeTessellateOp*>(grOp);

    if (fNeedsStencil ||
        op->fNeedsStencil ||
        fColor != op->fColor ||
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
        combinedFlags |= ShaderFlags::kDynamicStroke;
    }
    if (combinedFlags & ShaderFlags::kDynamicStroke) {
        // Don't actually enable dynamic stroke on ops that already have lots of verbs.
        if (!this->shouldUseDynamicState(ShaderFlags::kDynamicStroke) ||
            !op->shouldUseDynamicState(ShaderFlags::kDynamicStroke)) {
            return CombineResult::kCannotCombine;
        }
    }

    fPathStrokeList.concat(std::move(op->fPathStrokeList), alloc);
    fTotalCombinedVerbCnt += op->fTotalCombinedVerbCnt;
    fShaderFlags = combinedFlags;
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

void GrStrokeTessellateOp::prePrepareTessellator(GrPathShader::ProgramArgs&& args,
                                                 GrAppliedClip&& clip) {
    SkASSERT(!fTessellator);
    SkASSERT(!fFillProgram);
    SkASSERT(!fStencilProgram);

    const GrCaps& caps = *args.fCaps;
    SkArenaAlloc* arena = args.fArena;

    // Only use hardware tessellation if the path has a somewhat large number of verbs. Otherwise we
    // seem to be better off using indirect draws. Our back door for HW tessellation shaders isn't
    // currently capable of passing varyings to the fragment shader either, so if the processors
    // have varyings we need to use indirect draws.
    GrStrokeTessellateShader::Mode shaderMode;
    if (caps.shaderCaps()->tessellationSupport() &&
        fTotalCombinedVerbCnt > 50 &&
        !fProcessors.usesVaryingCoords()) {
        fTessellator = arena->make<GrStrokeHardwareTessellator>(fShaderFlags, *caps.shaderCaps());
        shaderMode = GrStrokeTessellateShader::Mode::kTessellation;
    } else {
        fTessellator = arena->make<GrStrokeIndirectTessellator>(fShaderFlags, fViewMatrix,
                                                                fPathStrokeList,
                                                                fTotalCombinedVerbCnt, arena);
        shaderMode = GrStrokeTessellateShader::Mode::kIndirect;
    }

    // If we are mixed sampled then we need a separate pipeline for the stencil pass. This is
    // because mixed samples either needs conservative raster enabled or MSAA disabled during fill.
    const GrPipeline* mixedSampledStencilPipeline = nullptr;
    if (fAAType == GrAAType::kCoverage) {
        SkASSERT(args.fWriteView.asRenderTargetProxy()->numSamples() == 1);
        SkASSERT(fNeedsStencil);  // Mixed samples always needs stencil.
        mixedSampledStencilPipeline = GrStencilPathShader::MakeStencilPassPipeline(
                args, fAAType, GrTessellationPathRenderer::OpFlags::kNone, clip.hardClip());
    }

    auto* strokeTessellateShader = arena->make<GrStrokeTessellateShader>(
            shaderMode, fShaderFlags, fViewMatrix, this->headStroke(), fColor);
    auto* fillPipeline = GrFillPathShader::MakeFillPassPipeline(args, fAAType, std::move(clip),
                                                                std::move(fProcessors));
    auto fillStencil = &GrUserStencilSettings::kUnused;
    if (fNeedsStencil) {
        auto* stencilPipeline = (mixedSampledStencilPipeline) ? mixedSampledStencilPipeline
                                                              : fillPipeline;
        fStencilProgram = GrPathShader::MakeProgram(args, strokeTessellateShader, stencilPipeline,
                                                    &kMarkStencil);
        fillStencil = &kTestAndResetStencil;
        args.fXferBarrierFlags = GrXferBarrierFlags::kNone;
    }

    fFillProgram = GrPathShader::MakeProgram(args, strokeTessellateShader, fillPipeline,
                                             fillStencil);
}

void GrStrokeTessellateOp::onPrePrepare(GrRecordingContext* context,
                                        const GrSurfaceProxyView& writeView, GrAppliedClip* clip,
                                        const GrXferProcessor::DstProxyView& dstProxyView,
                                        GrXferBarrierFlags renderPassXferBarriers, GrLoadOp
                                        colorLoadOp) {
    this->prePrepareTessellator({context->priv().recordTimeAllocator(), writeView, &dstProxyView,
                                renderPassXferBarriers, colorLoadOp, context->priv().caps()},
                                (clip) ? std::move(*clip) : GrAppliedClip::Disabled());
    if (fStencilProgram) {
        context->priv().recordProgramInfo(fStencilProgram);
    }
    if (fFillProgram) {
        context->priv().recordProgramInfo(fFillProgram);
    }
}

void GrStrokeTessellateOp::onPrepare(GrOpFlushState* flushState) {
    if (!fTessellator) {
        this->prePrepareTessellator({flushState->allocator(), flushState->writeView(),
                                    &flushState->dstProxyView(), flushState->renderPassBarriers(),
                                    flushState->colorLoadOp(), &flushState->caps()},
                                    flushState->detachAppliedClip());
    }
    SkASSERT(fTessellator);
    fTessellator->prepare(flushState, fViewMatrix, fPathStrokeList, fTotalCombinedVerbCnt);
}

void GrStrokeTessellateOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
    SkASSERT(chainBounds == this->bounds());
    if (fStencilProgram) {
        flushState->bindPipelineAndScissorClip(*fStencilProgram, this->bounds());
        flushState->bindTextures(fStencilProgram->primProc(), nullptr, fStencilProgram->pipeline());
        fTessellator->draw(flushState);
    }
    if (fFillProgram) {
        flushState->bindPipelineAndScissorClip(*fFillProgram, this->bounds());
        flushState->bindTextures(fFillProgram->primProc(), nullptr, fFillProgram->pipeline());
        fTessellator->draw(flushState);
    }
}
