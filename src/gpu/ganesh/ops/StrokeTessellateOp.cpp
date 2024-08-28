/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/ops/StrokeTessellateOp.h"

#include "include/core/SkColor.h"
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/gpu/ganesh/GrRecordingContext.h"
#include "src/base/SkArenaAlloc.h"
#include "src/gpu/ganesh/GrAppliedClip.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrOpFlushState.h"
#include "src/gpu/ganesh/GrPaint.h"
#include "src/gpu/ganesh/GrProcessorAnalysis.h"
#include "src/gpu/ganesh/GrProgramInfo.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrRenderTargetProxy.h"
#include "src/gpu/ganesh/GrShaderCaps.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/GrUserStencilSettings.h"
#include "src/gpu/ganesh/tessellate/GrStrokeTessellationShader.h"

#include <utility>

namespace skgpu::ganesh {

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
        fPatchAttribs |= PatchAttribs::kWideColorIfEnabled;
    }
    SkRect devBounds = path.getBounds();
    if (!this->headStroke().isHairlineStyle()) {
        // Non-hairlines inflate in local path space (pre-transform).
        float r = stroke.getInflationRadius();
        devBounds.outset(r, r);
    }
    viewMatrix.mapRect(&devBounds, devBounds);
    if (this->headStroke().isHairlineStyle()) {
        // Hairlines inflate in device space (post-transform).
        float r = SkStrokeRec::GetInflationRadius(stroke.getJoin(), stroke.getMiter(),
                                                  stroke.getCap(), 1);
        devBounds.outset(r, r);
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
    if (!caps.shaderCaps()->fInfinitySupport) {
        // The GPU can't infer curve type based in infinity, so we need to send in an attrib
        // explicitly stating the curve type.
        fPatchAttribs |= PatchAttribs::kExplicitCurveType;
    }
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

    auto combinedAttribs = fPatchAttribs | op->fPatchAttribs;
    if (!(combinedAttribs & PatchAttribs::kStrokeParams) &&
        !tess::StrokesHaveEqualParams(this->headStroke(), op->headStroke())) {
        // The paths have different stroke properties. We will need to enable dynamic stroke if we
        // still decide to combine them.
        if (this->headStroke().isHairlineStyle()) {
            return CombineResult::kCannotCombine;  // Dynamic hairlines aren't supported.
        }
        combinedAttribs |= PatchAttribs::kStrokeParams;
    }
    if (!(combinedAttribs & PatchAttribs::kColor) && this->headColor() != op->headColor()) {
        // The paths have different colors. We will need to enable dynamic color if we still decide
        // to combine them.
        combinedAttribs |= PatchAttribs::kColor;
    }

    // Don't actually enable new dynamic state on ops that already have lots of verbs.
    constexpr static SkTFlagsMask<PatchAttribs> kDynamicStatesMask(PatchAttribs::kStrokeParams |
                                                                   PatchAttribs::kColor);
    PatchAttribs neededDynamicStates = combinedAttribs & kDynamicStatesMask;
    if (neededDynamicStates != PatchAttribs::kNone) {
        if (!this->shouldUseDynamicStates(neededDynamicStates) ||
            !op->shouldUseDynamicStates(neededDynamicStates)) {
            return CombineResult::kCannotCombine;
        }
    }

    fPatchAttribs = combinedAttribs;

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

    auto* pipeline = GrTessellationShader::MakePipeline(args, fAAType, std::move(clip),
                                                        std::move(fProcessors));

    fTessellator = arena->make<StrokeTessellator>(fPatchAttribs);
    fTessellationShader = args.fArena->make<GrStrokeTessellationShader>(
            *caps.shaderCaps(),
            fPatchAttribs,
            fViewMatrix,
            this->headStroke(),
            this->headColor());

    auto fillStencil = &GrUserStencilSettings::kUnused;
    if (fNeedsStencil) {
        fStencilProgram = GrTessellationShader::MakeProgram(args, fTessellationShader, pipeline,
                                                            &kMarkStencil);
        fillStencil = &kTestAndResetStencil;
        // TODO: Currently if we have a texture barrier for a dst read it will get put in before
        // both the stencil draw and the fill draw. In reality we only really need the barrier
        // once to guard the reads of the color buffer in the fill from the previous writes. Maybe
        // we can investigate how to remove one of these barriers but it is probably not something
        // that is required a lot and thus the extra barrier shouldn't be too much of a perf hit to
        // general Skia use.
    }

    fFillProgram = GrTessellationShader::MakeProgram(args, fTessellationShader, pipeline,
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
    fTessellator->prepare(flushState,
                          fViewMatrix,
                          &fPathStrokeList,
                          fTotalCombinedVerbCnt);
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

}  // namespace skgpu::ganesh
