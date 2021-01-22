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
                                                       bool hasMixedSampledCoverage,
                                                       GrClampType clampType) {
    return fProcessors.finalize(fColor, GrProcessorAnalysisCoverage::kNone, clip, nullptr,
                                hasMixedSampledCoverage, caps, clampType, &fColor);
}

const GrProgramInfo* make_fill_bbox_program(const GrPathShader::ProgramArgs& args,
                                            const SkMatrix& viewMatrix, const SkPMColor4f& color,
                                            const SkRect& pathBounds, GrAAType aaType,
                                            GrAppliedClip&& appliedClip,
                                            GrProcessorSet&& processors) {
    // Allows non-zero stencil values to pass and write a color, and resets the stencil value back
    // to zero; discards immediately on stencil values of zero.
    // NOTE: It's ok to not check the clip here because the previous stencil pass will have only
    // written to samples already inside the clip.
    constexpr static GrUserStencilSettings kTestAndResetStencil(
        GrUserStencilSettings::StaticInit<
            0x0000,
            GrUserStencilTest::kNotEqual,
            0xffff,
            GrUserStencilOp::kZero,
            GrUserStencilOp::kKeep,
            0xffff>());

    auto* shader = args.fArena->make<GrFillBoundingBoxShader>(viewMatrix, color, pathBounds);

    auto pipelineFlags = GrPipeline::InputFlags::kNone;
    if (aaType != GrAAType::kNone) {
        if (args.fWriteView.asRenderTargetProxy()->numSamples() == 1) {
            // We are mixed sampled. We need to either enable conservative raster (preferred) or
            // disable MSAA in order to avoid double blend artifacts. (Even if we disable MSAA for
            // the fill geometry, the stencil test is still multisampled and will still produce
            // smooth results.)
            SkASSERT(aaType == GrAAType::kCoverage);
            if (args.fCaps->conservativeRasterSupport()) {
                pipelineFlags |= GrPipeline::InputFlags::kHWAntialias |
                                 GrPipeline::InputFlags::kConservativeRaster;
            }
        } else {
            // We are standard MSAA. Leave MSAA enabled for the fill geometry.
            pipelineFlags |= GrPipeline::InputFlags::kHWAntialias;
        }
    }

    return GrPathShader::MakeProgram(args, shader, std::move(appliedClip), std::move(processors),
                                     pipelineFlags, &kTestAndResetStencil);
}

void GrPathStencilFillOp::prePreparePrograms(const GrPathShader::ProgramArgs& args,
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
    const auto* stencilPassSettings = GrStencilPathShader::StencilPassSettings(fPath.getFillType());
    if (drawTrianglesAsIndirectCurveDraw || (fOpFlags & OpFlags::kDisableHWTessellation)) {
        fTessellator = args.fArena->make<GrPathIndirectTessellator>(
                fViewMatrix, fPath, DrawInnerFan(drawTrianglesAsIndirectCurveDraw));
        if (!drawTrianglesAsIndirectCurveDraw) {
            fStencilFanProgram = GrStencilPathShader::MakeStencilProgram<GrStencilTriangleShader>(
                    args, fViewMatrix, stencilPassPipeline, stencilPassSettings);
        }
        fStencilPathProgram = GrStencilPathShader::MakeStencilProgram<GrMiddleOutCubicShader>(
                args, fViewMatrix, stencilPassPipeline, stencilPassSettings);
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
                    args, fViewMatrix, stencilPassPipeline, stencilPassSettings);
            fStencilPathProgram = GrStencilPathShader::MakeStencilProgram<GrCubicTessellateShader>(
                    args, fViewMatrix, stencilPassPipeline, stencilPassSettings);
        } else {
            // Fastest CPU approach: emit one cubic wedge per verb, fanning out from the center.
            fTessellator = args.fArena->make<GrPathWedgeTessellator>();
            fStencilPathProgram = GrStencilPathShader::MakeStencilProgram<GrWedgeTessellateShader>(
                    args, fViewMatrix, stencilPassPipeline, stencilPassSettings);
        }
    }

    if (!(fOpFlags & OpFlags::kStencilOnly)) {
        fFillBBoxProgram = make_fill_bbox_program(args, fViewMatrix, fColor, fPath.getBounds(),
                                                  fAAType, std::move(appliedClip),
                                                  std::move(fProcessors));
    }
}

void GrPathStencilFillOp::onPrePrepare(GrRecordingContext* context,
                                       const GrSurfaceProxyView& writeView, GrAppliedClip* clip,
                                       const GrXferProcessor::DstProxyView& dstProxyView,
                                       GrXferBarrierFlags renderPassXferBarriers,
                                       GrLoadOp colorLoadOp) {
    this->prePreparePrograms({context->priv().recordTimeAllocator(), writeView, &dstProxyView,
                             renderPassXferBarriers, colorLoadOp, context->priv().caps()},
                             std::move(*clip));
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
        auto* triangleVertexData = vertexAlloc.lock<SkPoint>(maxFanTriangles * 3);
        fFanVertexCount = GrMiddleOutPolygonTriangulator::WritePathInnerFan(
                triangleVertexData, 3/*perTriangleVertexAdvance*/, fPath) * 3;
        SkASSERT(fFanVertexCount <= maxFanTriangles * 3);
        vertexAlloc.unlock(fFanVertexCount);
    }

    fTessellator->prepare(flushState, fViewMatrix, fPath);
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
        flushState->bindTextures(fFillBBoxProgram->primProc(), nullptr,
                                 fFillBBoxProgram->pipeline());
        flushState->bindBuffers(nullptr, nullptr, nullptr);
        flushState->draw(4, 0);
    }
}
