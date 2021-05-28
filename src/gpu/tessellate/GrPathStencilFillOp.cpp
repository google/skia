/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrPathStencilFillOp.h"

#include "src/gpu/GrEagerVertexAllocator.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/tessellate/GrFillPathShader.h"
#include "src/gpu/tessellate/GrMiddleOutPolygonTriangulator.h"
#include "src/gpu/tessellate/GrPathTessellator.h"
#include "src/gpu/tessellate/GrStencilPathShader.h"
#include "src/gpu/tessellate/GrTessellationPathRenderer.h"

using OpFlags = GrTessellationPathRenderer::OpFlags;

void GrPathStencilFillOp::visitProxies(const VisitProxyFunc& fn) const {
    if (fFillBBoxProgram) {
        fFillBBoxProgram->pipeline().visitProxies(fn);
    } else {
        fProcessors.visitProxies(fn);
    }
}

GrDrawOp::FixedFunctionFlags GrPathStencilFillOp::fixedFunctionFlags() const {
    auto flags = FixedFunctionFlags::kUsesStencil;
    if (fAAType != GrAAType::kNone) {
        flags |= FixedFunctionFlags::kUsesHWAA;
    }
    return flags;
}

GrProcessorSet::Analysis GrPathStencilFillOp::finalize(const GrCaps& caps,
                                                       const GrAppliedClip* clip,
                                                       GrClampType clampType) {
    return fProcessors.finalize(fColor, GrProcessorAnalysisCoverage::kNone, clip, nullptr, caps,
                                clampType, &fColor);
}

void GrPathStencilFillOp::prePreparePrograms(const GrPathShader::ProgramArgs& args,
                                             GrAppliedClip&& appliedClip) {
    SkASSERT(!fTessellator);
    SkASSERT(!fStencilFanProgram);
    SkASSERT(!fStencilPathProgram);
    SkASSERT(!fFillBBoxProgram);

    if (fPath.countVerbs() <= 0) {
        return;
    }

    const GrPipeline* stencilPassPipeline = GrStencilPathShader::MakeStencilPassPipeline(
            args, fAAType, fOpFlags, appliedClip.hardClip());

    if ((fOpFlags & OpFlags::kPreferWedges) && args.fCaps->shaderCaps()->tessellationSupport()) {
        // The path is an atlas with relatively small contours, or something else that does best
        // with wedges.
        fTessellator = GrPathWedgeTessellator::Make(args.fArena, fViewMatrix);
    } else {
        auto drawFanWithTessellator = GrPathTessellator::DrawInnerFan::kYes;
        if (fPath.countVerbs() > 50 && this->bounds().height() * this->bounds().width() > 256*256) {
            // Large complex paths do better with a dedicated triangle shader for the inner fan.
            // This takes less PCI bus bandwidth (6 floats per triangle instead of 8) and allows us
            // to make sure it has an efficient middle-out topology.
            fStencilFanProgram = GrStencilPathShader::MakeStencilProgram<GrStencilTriangleShader>(
                    args, fViewMatrix, stencilPassPipeline, fPath.getFillType());
            drawFanWithTessellator = GrPathTessellator::DrawInnerFan::kNo;
        }
        fTessellator = GrPathTessellator::Make(args.fArena, fViewMatrix, fPath,
                                               drawFanWithTessellator, *args.fCaps);
    }

    fStencilPathProgram = GrPathShader::MakeProgram(
            args, fTessellator->shader(), stencilPassPipeline,
            GrStencilPathShader::StencilPassSettings(fPath.getFillType()));

    if (!(fOpFlags & OpFlags::kStencilOnly)) {
        // Create a program that draws a bounding box over the path and fills its stencil coverage
        // into the color buffer.
        auto* bboxShader = args.fArena->make<GrFillBoundingBoxShader>(fViewMatrix, fColor,
                                                                      fPath.getBounds());
        auto* bboxPipeline = GrFillPathShader::MakeFillPassPipeline(args, fAAType,
                                                                    std::move(appliedClip),
                                                                    std::move(fProcessors));
        auto* bboxStencil = GrFillPathShader::TestAndResetStencilSettings();
        fFillBBoxProgram = GrPathShader::MakeProgram(args, bboxShader, bboxPipeline, bboxStencil);
    }
}

void GrPathStencilFillOp::onPrePrepare(GrRecordingContext* context,
                                       const GrSurfaceProxyView& writeView, GrAppliedClip* clip,
                                       const GrXferProcessor::DstProxyView& dstProxyView,
                                       GrXferBarrierFlags renderPassXferBarriers,
                                       GrLoadOp colorLoadOp) {
    this->prePreparePrograms({context->priv().recordTimeAllocator(), writeView, &dstProxyView,
                             renderPassXferBarriers, colorLoadOp, context->priv().caps()},
                             (clip) ? std::move(*clip) : GrAppliedClip::Disabled());
    if (fStencilFanProgram) {
        context->priv().recordProgramInfo(fStencilFanProgram);
    }
    if (fStencilPathProgram) {
        context->priv().recordProgramInfo(fStencilPathProgram);
    }
    if (fFillBBoxProgram) {
        context->priv().recordProgramInfo(fFillBBoxProgram);
    }
}

void GrPathStencilFillOp::onPrepare(GrOpFlushState* flushState) {
    if (!fTessellator) {
        this->prePreparePrograms({flushState->allocator(), flushState->writeView(),
                                  &flushState->dstProxyView(), flushState->renderPassBarriers(),
                                  flushState->colorLoadOp(), &flushState->caps()},
                                  flushState->detachAppliedClip());
        if (!fTessellator) {
            return;
        }
    }

    if (fStencilFanProgram) {
        // The inner fan isn't built into the tessellator. Generate a standard Redbook fan with a
        // middle-out topology.
        GrEagerDynamicVertexAllocator vertexAlloc(flushState, &fFanBuffer, &fFanBaseVertex);
        int maxFanTriangles = fPath.countVerbs() - 2;  // n - 2 triangles make an n-gon.
        GrVertexWriter triangleVertexWriter = vertexAlloc.lock<SkPoint>(maxFanTriangles * 3);
        fFanVertexCount = GrMiddleOutPolygonTriangulator::WritePathInnerFan(
                &triangleVertexWriter, GrMiddleOutPolygonTriangulator::OutputType::kTriangles,
                fPath) * 3;
        SkASSERT(fFanVertexCount <= maxFanTriangles * 3);
        vertexAlloc.unlock(fFanVertexCount);
    }

    fTessellator->prepare(flushState, this->bounds(), fPath);

    if (fFillBBoxProgram) {
        GrVertexWriter vertexWriter = flushState->makeVertexSpace(sizeof(SkRect), 1, &fBBoxBuffer,
                                                                  &fBBoxBaseInstance);
        vertexWriter.write(fPath.getBounds());
    }
}

void GrPathStencilFillOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
    if (!fTessellator) {
        return;
    }

    // Stencil the inner fan, if any.
    if (fFanVertexCount > 0) {
        SkASSERT(fStencilFanProgram);
        SkASSERT(fFanBuffer);
        flushState->bindPipelineAndScissorClip(*fStencilFanProgram, this->bounds());
        flushState->bindBuffers(nullptr, nullptr, fFanBuffer);
        flushState->draw(fFanVertexCount, fFanBaseVertex);
    }

    // Stencil the rest of the path.
    SkASSERT(fStencilPathProgram);
    flushState->bindPipelineAndScissorClip(*fStencilPathProgram, this->bounds());
    fTessellator->draw(flushState);

    // Fill in the bounding box (if not in stencil-only mode).
    if (fFillBBoxProgram) {
        flushState->bindPipelineAndScissorClip(*fFillBBoxProgram, this->bounds());
        flushState->bindTextures(fFillBBoxProgram->geomProc(), nullptr,
                                 fFillBBoxProgram->pipeline());
        flushState->bindBuffers(nullptr, fBBoxBuffer, nullptr);
        flushState->drawInstanced(1, fBBoxBaseInstance, 4, 0);
    }
}
