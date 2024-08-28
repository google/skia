/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/ganesh/ops/PathTessellateOp.h"

#include "include/core/SkColor.h"
#include "include/gpu/ganesh/GrRecordingContext.h"
#include "src/gpu/ganesh/GrAppliedClip.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrOpFlushState.h"
#include "src/gpu/ganesh/GrPipeline.h"
#include "src/gpu/ganesh/GrProcessorAnalysis.h"
#include "src/gpu/ganesh/GrProgramInfo.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrRenderTargetProxy.h"
#include "src/gpu/ganesh/GrShaderCaps.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/tessellate/GrPathTessellationShader.h"

namespace skgpu::ganesh {

void PathTessellateOp::visitProxies(const GrVisitProxyFunc& func) const {
    if (fTessellationProgram) {
        fTessellationProgram->pipeline().visitProxies(func);
    } else {
        fProcessors.visitProxies(func);
    }
}

GrProcessorSet::Analysis PathTessellateOp::finalize(const GrCaps& caps,
                                                    const GrAppliedClip* clip,
                                                    GrClampType clampType) {
    auto analysis = fProcessors.finalize(this->headDraw().fColor,
                                         GrProcessorAnalysisCoverage::kNone,
                                         clip,
                                         nullptr,
                                         caps,
                                         clampType,
                                         &this->headDraw().fColor);
    if (!analysis.usesLocalCoords()) {
        // Since we don't need local coords, we can transform on CPU instead of in the shader. This
        // gives us better batching potential.
        this->headDraw().fPathMatrix = fShaderMatrix;
        fShaderMatrix = SkMatrix::I();
    }
    return analysis;
}

GrDrawOp::CombineResult PathTessellateOp::onCombineIfPossible(GrOp* grOp,
                                                              SkArenaAlloc*,
                                                              const GrCaps&) {
    auto* op = grOp->cast<PathTessellateOp>();
    bool canMerge = fAAType == op->fAAType &&
                    fStencil == op->fStencil &&
                    fProcessors == op->fProcessors &&
                    fShaderMatrix == op->fShaderMatrix;
    if (canMerge) {
        fTotalCombinedPathVerbCnt += op->fTotalCombinedPathVerbCnt;
        fPatchAttribs |= op->fPatchAttribs;

        if (!(fPatchAttribs & PatchAttribs::kColor) &&
            this->headDraw().fColor != op->headDraw().fColor) {
            // Color is no longer uniform. Move it into patch attribs.
            fPatchAttribs |= PatchAttribs::kColor;
        }

        *fPathDrawTail = op->fPathDrawList;
        fPathDrawTail = op->fPathDrawTail;
        return CombineResult::kMerged;
    }

    return CombineResult::kCannotCombine;
}

void PathTessellateOp::prepareTessellator(const GrTessellationShader::ProgramArgs& args,
                                          GrAppliedClip&& appliedClip) {
    SkASSERT(!fTessellator);
    SkASSERT(!fTessellationProgram);
    auto* pipeline = GrTessellationShader::MakePipeline(args, fAAType, std::move(appliedClip),
                                                        std::move(fProcessors));
    fTessellator = PathWedgeTessellator::Make(args.fArena,
                                              args.fCaps->shaderCaps()->fInfinitySupport,
                                              fPatchAttribs);
    auto* tessShader = GrPathTessellationShader::Make(*args.fCaps->shaderCaps(),
                                                      args.fArena,
                                                      fShaderMatrix,
                                                      this->headDraw().fColor,
                                                      fTessellator->patchAttribs());
    fTessellationProgram = GrTessellationShader::MakeProgram(args, tessShader, pipeline, fStencil);
}

void PathTessellateOp::onPrePrepare(GrRecordingContext* context,
                                    const GrSurfaceProxyView& writeView, GrAppliedClip* clip,
                                    const GrDstProxyView& dstProxyView,
                                    GrXferBarrierFlags renderPassXferBarriers,
                                    GrLoadOp colorLoadOp) {
    // DMSAA is not supported on DDL.
    bool usesMSAASurface = writeView.asRenderTargetProxy()->numSamples() > 1;
    this->prepareTessellator({context->priv().recordTimeAllocator(), writeView, usesMSAASurface,
                             &dstProxyView, renderPassXferBarriers, colorLoadOp,
                             context->priv().caps()},
                             (clip) ? std::move(*clip) : GrAppliedClip::Disabled());
    SkASSERT(fTessellationProgram);
    context->priv().recordProgramInfo(fTessellationProgram);
}

void PathTessellateOp::onPrepare(GrOpFlushState* flushState) {
    if (!fTessellator) {
        this->prepareTessellator({flushState->allocator(), flushState->writeView(),
                                 flushState->usesMSAASurface(), &flushState->dstProxyView(),
                                 flushState->renderPassBarriers(), flushState->colorLoadOp(),
                                 &flushState->caps()}, flushState->detachAppliedClip());
        SkASSERT(fTessellator);
    }
    fTessellator->prepare(flushState,
                          fShaderMatrix,
                          *fPathDrawList,
                          fTotalCombinedPathVerbCnt);
}

void PathTessellateOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
    SkASSERT(fTessellator);
    SkASSERT(fTessellationProgram);
    flushState->bindPipelineAndScissorClip(*fTessellationProgram, this->bounds());
    flushState->bindTextures(fTessellationProgram->geomProc(), nullptr,
                             fTessellationProgram->pipeline());
    fTessellator->draw(flushState);
}

}  // namespace skgpu::ganesh
