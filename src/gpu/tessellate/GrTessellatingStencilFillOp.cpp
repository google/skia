/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrTessellatingStencilFillOp.h"

#include "src/gpu/GrEagerVertexAllocator.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/tessellate/GrFillPathShader.h"
#include "src/gpu/tessellate/GrMiddleOutPolygonTriangulator.h"
#include "src/gpu/tessellate/GrPathTessellator.h"
#include "src/gpu/tessellate/GrStencilPathShader.h"
#include "src/gpu/tessellate/GrTessellationPathRenderer.h"

using OpFlags = GrTessellationPathRenderer::OpFlags;

void GrTessellatingStencilFillOp::visitProxies(const VisitProxyFunc& fn) const {
    if (fFillBBoxProgram) {
        fFillBBoxProgram->pipeline().visitProxies(fn);
    } else {
        fProcessors.visitProxies(fn);
    }
}

GrDrawOp::FixedFunctionFlags GrTessellatingStencilFillOp::fixedFunctionFlags() const {
    auto flags = FixedFunctionFlags::kUsesStencil;
    if (fAAType != GrAAType::kNone) {
        flags |= FixedFunctionFlags::kUsesHWAA;
    }
    return flags;
}

GrProcessorSet::Analysis GrTessellatingStencilFillOp::finalize(const GrCaps& caps,
                                                               const GrAppliedClip* clip,
                                                               bool hasMixedSampledCoverage,
                                                               GrClampType clampType) {
    return fProcessors.finalize(fColor, GrProcessorAnalysisCoverage::kNone, clip, nullptr,
                                hasMixedSampledCoverage, caps, clampType, &fColor);
}

void GrTessellatingStencilFillOp::prePreparePrograms(const GrPathShader::ProgramArgs& args,
                                                     GrAppliedClip&& appliedClip) {
    using DrawInnerFan = GrPathIndirectTessellator::DrawInnerFan;
    SkASSERT(!fStencilFanProgram);
    SkASSERT(!fStencilPathProgram);
    SkASSERT(!fFillBBoxProgram);

    int numVerbs = fPath.countVerbs();
    if (numVerbs <= 0) {
        return;
    }

    // When there are only a few verbs, it seems to always be fastest to make a single indirect draw
    // that contains both the inner triangles and the outer curves, instead of using hardware
    // tessellation. Also take this path if tessellation is not supported.
    bool drawTrianglesAsIndirectCurveDraw = (numVerbs < 50);
    const GrPipeline* stencilPassPipeline = GrStencilPathShader::MakeStencilPassPipeline(
            args, fAAType, fOpFlags, appliedClip.hardClip());
    if (drawTrianglesAsIndirectCurveDraw || (fOpFlags & OpFlags::kDisableHWTessellation)) {
        fTessellator = args.fArena->make<GrPathIndirectTessellator>(
                fViewMatrix, fPath, DrawInnerFan(drawTrianglesAsIndirectCurveDraw));
        if (!drawTrianglesAsIndirectCurveDraw) {
            fStencilFanProgram = GrStencilPathShader::MakeStencilProgram<GrStencilTriangleShader>(
                    args, fViewMatrix, stencilPassPipeline, fPath.getFillType());
        }
        fStencilPathProgram = GrStencilPathShader::MakeStencilProgram<GrMiddleOutCubicShader>(
                args, fViewMatrix, stencilPassPipeline, fPath.getFillType());
    } else {
        // The caller should have sent Flags::kDisableHWTessellation if it was not supported.
        SkASSERT(args.fCaps->shaderCaps()->tessellationSupport());
        // Next see if we can split up the inner triangles and outer curves into two draw calls.
        // This allows for a more efficient inner fan topology that can reduce the rasterizer load
        // by a large margin on complex paths, but also causes greater CPU overhead due to the extra
        // shader switches and draw calls.
        // NOTE: Raster-edge work is 1-dimensional, so we sum height and width instead of
        // multiplying.
        SkScalar scales[2];
        SkAssertResult(fViewMatrix.getMinMaxScales(scales));  // Will fail if perspective.
        const SkRect& bounds = fPath.getBounds();
        float rasterEdgeWork = (bounds.height() + bounds.width()) * scales[1] * fPath.countVerbs();
        if (rasterEdgeWork > 300 * 300) {
            fTessellator = args.fArena->make<GrPathOuterCurveTessellator>();
            fStencilFanProgram = GrStencilPathShader::MakeStencilProgram<GrStencilTriangleShader>(
                    args, fViewMatrix, stencilPassPipeline, fPath.getFillType());
            fStencilPathProgram = GrStencilPathShader::MakeStencilProgram<GrCubicTessellateShader>(
                    args, fViewMatrix, stencilPassPipeline, fPath.getFillType());
        } else {
            // Fastest CPU approach: emit one cubic wedge per verb, fanning out from the center.
            fTessellator = args.fArena->make<GrPathWedgeTessellator>();
            fStencilPathProgram = GrStencilPathShader::MakeStencilProgram<GrWedgeTessellateShader>(
                    args, fViewMatrix, stencilPassPipeline, fPath.getFillType());
        }
    }

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

void GrTessellatingStencilFillOp::onPrePrepare(GrRecordingContext* context,
                                               const GrSurfaceProxyView& writeView,
                                               GrAppliedClip* clip,
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

void GrTessellatingStencilFillOp::onPrepare(GrOpFlushState* flushState) {
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
        auto* triangleVertexData = vertexAlloc.lock<SkPoint>(maxFanTriangles * 3);
        fFanVertexCount = GrMiddleOutPolygonTriangulator::WritePathInnerFan(
                triangleVertexData, 3/*perTriangleVertexAdvance*/, fPath) * 3;
        SkASSERT(fFanVertexCount <= maxFanTriangles * 3);
        vertexAlloc.unlock(fFanVertexCount);
    }

    fTessellator->prepare(flushState, fViewMatrix, fPath);
}

void GrTessellatingStencilFillOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
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
        flushState->bindTextures(fFillBBoxProgram->primProc(), nullptr,
                                 fFillBBoxProgram->pipeline());
        flushState->bindBuffers(nullptr, nullptr, nullptr);
        flushState->draw(4, 0);
    }
}
