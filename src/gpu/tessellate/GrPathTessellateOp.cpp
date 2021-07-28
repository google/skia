/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrPathTessellateOp.h"

#include "src/gpu/tessellate/GrPathWedgeTessellator.h"
#include "src/gpu/tessellate/shaders/GrPathTessellationShader.h"

void GrPathTessellateOp::visitProxies(const GrVisitProxyFunc& func) const {
    if (fTessellationProgram) {
        fTessellationProgram->pipeline().visitProxies(func);
    } else {
        fProcessors.visitProxies(func);
    }
}

GrProcessorSet::Analysis GrPathTessellateOp::finalize(const GrCaps& caps,
                                                        const GrAppliedClip* clip,
                                                        GrClampType clampType) {
    return fProcessors.finalize(fColor, GrProcessorAnalysisCoverage::kNone, clip, nullptr, caps,
                                clampType, &fColor);
}

void GrPathTessellateOp::prepareTessellator(const GrTessellationShader::ProgramArgs& args,
                                            GrAppliedClip&& appliedClip) {
    SkASSERT(!fTessellator);
    SkASSERT(!fTessellationProgram);
    auto* pipeline = GrTessellationShader::MakePipeline(args, fAAType, std::move(appliedClip),
                                                        std::move(fProcessors));
    fTessellator = GrPathWedgeTessellator::Make(args.fArena, fViewMatrix, fColor,
                                                fPath.countVerbs(), *pipeline, *args.fCaps);
    fTessellationProgram = GrTessellationShader::MakeProgram(args, fTessellator->shader(), pipeline,
                                                             fStencil);
}

void GrPathTessellateOp::onPrePrepare(GrRecordingContext* context,
                                      const GrSurfaceProxyView& writeView, GrAppliedClip* clip,
                                      const GrDstProxyView& dstProxyView,
                                      GrXferBarrierFlags renderPassXferBarriers,
                                      GrLoadOp colorLoadOp) {
    this->prepareTessellator({context->priv().recordTimeAllocator(), writeView, &dstProxyView,
                             renderPassXferBarriers, colorLoadOp, context->priv().caps()},
                             (clip) ? std::move(*clip) : GrAppliedClip::Disabled());
    SkASSERT(fTessellationProgram);
    context->priv().recordProgramInfo(fTessellationProgram);
}

void GrPathTessellateOp::onPrepare(GrOpFlushState* flushState) {
    if (!fTessellator) {
        this->prepareTessellator({flushState->allocator(), flushState->writeView(),
                                  &flushState->dstProxyView(), flushState->renderPassBarriers(),
                                  flushState->colorLoadOp(), &flushState->caps()},
                                  flushState->detachAppliedClip());
        SkASSERT(fTessellator);
    }
    fTessellator->prepare(flushState, this->bounds(), {SkMatrix::I(), fPath}, fPath.countVerbs());
}

void GrPathTessellateOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
    SkASSERT(fTessellator);
    SkASSERT(fTessellationProgram);
    flushState->bindPipelineAndScissorClip(*fTessellationProgram, this->bounds());
    flushState->bindTextures(fTessellationProgram->geomProc(), nullptr,
                             fTessellationProgram->pipeline());
    fTessellator->draw(flushState);
}
