/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/PathTessellateOp.h"

#include "src/gpu/GrAppliedClip.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/tessellate/PathWedgeTessellator.h"
#include "src/gpu/tessellate/shaders/GrPathTessellationShader.h"

namespace skgpu::v1 {

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
    return fProcessors.finalize(fColor, GrProcessorAnalysisCoverage::kNone, clip, nullptr, caps,
                                clampType, &fColor);
}

void PathTessellateOp::prepareTessellator(const GrTessellationShader::ProgramArgs& args,
                                          GrAppliedClip&& appliedClip) {
    SkASSERT(!fTessellator);
    SkASSERT(!fTessellationProgram);
    auto* pipeline = GrTessellationShader::MakePipeline(args, fAAType, std::move(appliedClip),
                                                        std::move(fProcessors));
    fTessellator = skgpu::tess::PathWedgeTessellator::Make(args.fArena, fViewMatrix, fColor,
                                                           fPath.countVerbs(), *pipeline,
                                                           *args.fCaps);
    fTessellationProgram = GrTessellationShader::MakeProgram(args, fTessellator->shader(), pipeline,
                                                             fStencil);
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
    fTessellator->prepare(flushState, this->bounds(), {SkMatrix::I(), fPath}, fPath.countVerbs());
}

void PathTessellateOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
    SkASSERT(fTessellator);
    SkASSERT(fTessellationProgram);
    flushState->bindPipelineAndScissorClip(*fTessellationProgram, this->bounds());
    flushState->bindTextures(fTessellationProgram->geomProc(), nullptr,
                             fTessellationProgram->pipeline());

    fTessellator->draw(flushState);
}

} // namespace skgpu::v1
