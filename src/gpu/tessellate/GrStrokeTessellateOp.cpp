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
#include "src/gpu/tessellate/GrStrokeFixedCountTessellator.h"
#include "src/gpu/tessellate/GrStrokeHardwareTessellator.h"
#include "src/gpu/tessellate/GrStrokeIndirectTessellator.h"

using DynamicStroke = GrStrokeTessellateShader::DynamicStroke;

GrStrokeTessellateOp::GrStrokeTessellateOp(GrAAType aaType, const SkMatrix& viewMatrix,
                                           const SkPath& path, const SkStrokeRec& stroke,
                                           GrPaint&& paint)
        : GrDrawOp(ClassID())
        , fAAType(aaType)
        , fViewMatrix(viewMatrix)
        , fPathStrokeList(path, stroke, paint.getColor4f())
        , fTotalCombinedVerbCnt(path.countVerbs())
        , fProcessors(std::move(paint)) {
    if (SkPathPriv::ConicWeightCnt(path) != 0) {
        fShaderFlags |= ShaderFlags::kHasConics;
    }
    if (!this->headColor().fitsInBytes()) {
        fShaderFlags |= ShaderFlags::kWideColor;
    }
    SkRect devBounds = path.getBounds();
    float inflationRadius = stroke.getInflationRadius();
    devBounds.outset(inflationRadius, inflationRadius);
    viewMatrix.mapRect(&devBounds, devBounds);
    this->setBounds(devBounds, HasAABloat::kNo, IsHairline::kNo);
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
                                                        GrClampType clampType) {
    // Make sure the finalize happens before combining. We might change fNeedsStencil here.
    SkASSERT(fPathStrokeList.fNext == nullptr);
    const GrProcessorSet::Analysis& analysis = fProcessors.finalize(
            this->headColor(), GrProcessorAnalysisCoverage::kNone, clip,
            &GrUserStencilSettings::kUnused, caps, clampType, &this->headColor());
    fNeedsStencil = !analysis.unaffectedByDstValue();
    return analysis;
}

GrOp::CombineResult GrStrokeTessellateOp::onCombineIfPossible(GrOp* grOp, SkArenaAlloc* alloc,
                                                              const GrCaps& caps) {
    SkASSERT(grOp->classID() == this->classID());
    auto* op = static_cast<GrStrokeTessellateOp*>(grOp);

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

void GrStrokeTessellateOp::prePrepareTessellator(GrPathShader::ProgramArgs&& args,
                                                 GrAppliedClip&& clip) {
    SkASSERT(!fTessellator);
    SkASSERT(!fFillProgram);
    SkASSERT(!fStencilProgram);

    const GrCaps& caps = *args.fCaps;
    SkArenaAlloc* arena = args.fArena;

    if (fTotalCombinedVerbCnt > 50 && this->canUseHardwareTessellation(caps)) {
        // Only use hardware tessellation if we're drawing a somewhat large number of verbs.
        // Otherwise we seem to be better off using instanced draws.
        fTessellator = arena->make<GrStrokeHardwareTessellator>(fShaderFlags, fViewMatrix,
                                                                &fPathStrokeList,
                                                                *caps.shaderCaps());
    } else if (fTotalCombinedVerbCnt > 50 && !(fShaderFlags & ShaderFlags::kDynamicColor)) {
        // Only use the log2 indirect tessellator if we're drawing a somewhat large number of verbs
        // and the stroke doesn't use dynamic color. (The log2 indirect tessellator can't support
        // dynamic color without a z-buffer, due to how it reorders strokes.)
        fTessellator = arena->make<GrStrokeIndirectTessellator>(fShaderFlags, fViewMatrix,
                                                                &fPathStrokeList,
                                                                fTotalCombinedVerbCnt, arena);
    } else {
        fTessellator = arena->make<GrStrokeFixedCountTessellator>(fShaderFlags, fViewMatrix,
                                                                  &fPathStrokeList);
    }

    auto* pipeline = GrFillPathShader::MakeFillPassPipeline(args, fAAType, std::move(clip),
                                                            std::move(fProcessors));
    auto fillStencil = &GrUserStencilSettings::kUnused;
    if (fNeedsStencil) {
        fStencilProgram = GrPathShader::MakeProgram(args, fTessellator->shader(), pipeline,
                                                    &kMarkStencil);
        fillStencil = &kTestAndResetStencil;
        args.fXferBarrierFlags = GrXferBarrierFlags::kNone;
    }

    fFillProgram = GrPathShader::MakeProgram(args, fTessellator->shader(), pipeline, fillStencil);
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
    fTessellator->prepare(flushState, fTotalCombinedVerbCnt);
}

void GrStrokeTessellateOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
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
