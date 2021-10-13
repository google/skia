/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/PathStencilCoverOp.h"

#include "src/gpu/GrEagerVertexAllocator.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"
#include "src/gpu/ops/GrSimpleMeshDrawOpHelper.h"
#include "src/gpu/tessellate/MiddleOutPolygonTriangulator.h"
#include "src/gpu/tessellate/PathCurveTessellator.h"
#include "src/gpu/tessellate/PathWedgeTessellator.h"
#include "src/gpu/tessellate/shaders/GrPathTessellationShader.h"

namespace {

// Fills a path's bounding box, with subpixel outset to avoid possible T-junctions with extreme
// edges of the path.
// NOTE: The emitted geometry may not be axis-aligned, depending on the view matrix.
class BoundingBoxShader : public GrGeometryProcessor {
public:
    BoundingBoxShader(SkPMColor4f color, const GrShaderCaps& shaderCaps)
            : GrGeometryProcessor(kTessellate_BoundingBoxShader_ClassID)
            , fColor(color) {
        if (!shaderCaps.vertexIDSupport()) {
            constexpr static Attribute kUnitCoordAttrib("unitCoord", kFloat2_GrVertexAttribType,
                                                        kFloat2_GrSLType);
            this->setVertexAttributes(&kUnitCoordAttrib, 1);
        }
        constexpr static Attribute kInstanceAttribs[] = {
            {"matrix2d", kFloat4_GrVertexAttribType, kFloat4_GrSLType},
            {"translate", kFloat2_GrVertexAttribType, kFloat2_GrSLType},
            {"pathBounds", kFloat4_GrVertexAttribType, kFloat4_GrSLType}
        };
        this->setInstanceAttributes(kInstanceAttribs, SK_ARRAY_COUNT(kInstanceAttribs));
    }

private:
    const char* name() const final { return "tessellate_BoundingBoxShader"; }
    void addToKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const final {}
    std::unique_ptr<ProgramImpl> makeProgramImpl(const GrShaderCaps&) const final;

    const SkPMColor4f fColor;
};

std::unique_ptr<GrGeometryProcessor::ProgramImpl> BoundingBoxShader::makeProgramImpl(
        const GrShaderCaps&) const {
    class Impl : public ProgramImpl {
    public:
        void setData(const GrGLSLProgramDataManager& pdman,
                     const GrShaderCaps&,
                     const GrGeometryProcessor& gp) override {
            const SkPMColor4f& color = gp.cast<BoundingBoxShader>().fColor;
            pdman.set4f(fColorUniform, color.fR, color.fG, color.fB, color.fA);
        }

    private:
        void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) final {
            args.fVaryingHandler->emitAttributes(args.fGeomProc);

            // Vertex shader.
            if (args.fShaderCaps->vertexIDSupport()) {
                // If we don't have sk_VertexID support then "unitCoord" already came in as a vertex
                // attrib.
                args.fVertBuilder->codeAppend(R"(
                float2 unitCoord = float2(sk_VertexID & 1, sk_VertexID >> 1);)");
            }
            args.fVertBuilder->codeAppend(R"(
            // Bloat the bounding box by 1/4px to be certain we will reset every stencil value.
            float2x2 M_ = inverse(float2x2(matrix2d));
            float2 bloat = float2(abs(M_[0]) + abs(M_[1])) * .25;

            // Find the vertex position.
            float2 localcoord = mix(pathBounds.xy - bloat, pathBounds.zw + bloat, unitCoord);
            float2 vertexpos = float2x2(matrix2d) * localcoord + translate;)");
            gpArgs->fLocalCoordVar.set(kFloat2_GrSLType, "localcoord");
            gpArgs->fPositionVar.set(kFloat2_GrSLType, "vertexpos");

            // Fragment shader.
            const char* color;
            fColorUniform = args.fUniformHandler->addUniform(nullptr, kFragment_GrShaderFlag,
                                                             kHalf4_GrSLType, "color", &color);
            args.fFragBuilder->codeAppendf("half4 %s = %s;", args.fOutputColor, color);
            args.fFragBuilder->codeAppendf("const half4 %s = half4(1);", args.fOutputCoverage);
        }

        GrGLSLUniformHandler::UniformHandle fColorUniform;
    };

    return std::make_unique<Impl>();
}

}  // anonymous namespace

namespace skgpu::v1 {

void PathStencilCoverOp::visitProxies(const GrVisitProxyFunc& func) const {
    if (fCoverBBoxProgram) {
        fCoverBBoxProgram->pipeline().visitProxies(func);
    } else {
        fProcessors.visitProxies(func);
    }
}

GrDrawOp::FixedFunctionFlags PathStencilCoverOp::fixedFunctionFlags() const {
    auto flags = FixedFunctionFlags::kUsesStencil;
    if (fAAType != GrAAType::kNone) {
        flags |= FixedFunctionFlags::kUsesHWAA;
    }
    return flags;
}

GrProcessorSet::Analysis PathStencilCoverOp::finalize(const GrCaps& caps,
                                                      const GrAppliedClip* clip,
                                                      GrClampType clampType) {
    return fProcessors.finalize(fColor, GrProcessorAnalysisCoverage::kNone, clip, nullptr, caps,
                                clampType, &fColor);
}

void PathStencilCoverOp::prePreparePrograms(const GrTessellationShader::ProgramArgs& args,
                                            GrAppliedClip&& appliedClip) {
    SkASSERT(!fTessellator);
    SkASSERT(!fStencilFanProgram);
    SkASSERT(!fStencilPathProgram);
    SkASSERT(!fCoverBBoxProgram);

    // We transform paths on the CPU. This allows for better batching.
    const SkMatrix& shaderMatrix = SkMatrix::I();
    auto pipelineFlags = (fPathFlags & FillPathFlags::kWireframe)
            ? GrPipeline::InputFlags::kWireframe
            : GrPipeline::InputFlags::kNone;
    const GrPipeline* stencilPipeline = GrPathTessellationShader::MakeStencilOnlyPipeline(
            args, fAAType, appliedClip.hardClip(), pipelineFlags);
    const GrUserStencilSettings* stencilSettings = GrPathTessellationShader::StencilPathSettings(
                    GrFillRuleForPathFillType(this->pathFillType()));

    if (fTotalCombinedPathVerbCnt > 50 &&
        this->bounds().height() * this->bounds().width() > 256 * 256) {
        // Large complex paths do better with a dedicated triangle shader for the inner fan.
        // This takes less PCI bus bandwidth (6 floats per triangle instead of 8) and allows us
        // to make sure it has an efficient middle-out topology.
        auto shader = GrPathTessellationShader::MakeSimpleTriangleShader(args.fArena,
                                                                         shaderMatrix,
                                                                         SK_PMColor4fTRANSPARENT);
        fStencilFanProgram = GrTessellationShader::MakeProgram(args,
                                                               shader,
                                                               stencilPipeline,
                                                               stencilSettings);
        fTessellator = skgpu::tess::PathCurveTessellator::Make(
                args.fArena,
                shaderMatrix,
                SK_PMColor4fTRANSPARENT,
                skgpu::tess::PathCurveTessellator::DrawInnerFan::kNo,
                fTotalCombinedPathVerbCnt,
                *stencilPipeline,
                *args.fCaps);
    } else {
        fTessellator = skgpu::tess::PathWedgeTessellator::Make(args.fArena,
                                                               shaderMatrix,
                                                               SK_PMColor4fTRANSPARENT,
                                                               fTotalCombinedPathVerbCnt,
                                                               *stencilPipeline,
                                                               *args.fCaps);
    }
    fStencilPathProgram = GrTessellationShader::MakeProgram(args,
                                                            fTessellator->shader(),
                                                            stencilPipeline,
                                                            stencilSettings);

    if (!(fPathFlags & FillPathFlags::kStencilOnly)) {
        // Create a program that draws a bounding box over the path and fills its stencil coverage
        // into the color buffer.
        auto* bboxShader = args.fArena->make<BoundingBoxShader>(fColor, *args.fCaps->shaderCaps());
        auto* bboxPipeline = GrTessellationShader::MakePipeline(args, fAAType,
                                                                std::move(appliedClip),
                                                                std::move(fProcessors));
        auto* bboxStencil = GrPathTessellationShader::TestAndResetStencilSettings(
                SkPathFillType_IsInverse(this->pathFillType()));
        fCoverBBoxProgram = GrSimpleMeshDrawOpHelper::CreateProgramInfo(
                args.fCaps,
                args.fArena,
                bboxPipeline,
                args.fWriteView,
                args.fUsesMSAASurface,
                bboxShader,
                GrPrimitiveType::kTriangleStrip,
                args.fXferBarrierFlags,
                args.fColorLoadOp,
                bboxStencil);
    }
}

void PathStencilCoverOp::onPrePrepare(GrRecordingContext* context,
                                      const GrSurfaceProxyView& writeView, GrAppliedClip* clip,
                                      const GrDstProxyView& dstProxyView,
                                      GrXferBarrierFlags renderPassXferBarriers,
                                      GrLoadOp colorLoadOp) {
    // DMSAA is not supported on DDL.
    bool usesMSAASurface = writeView.asRenderTargetProxy()->numSamples() > 1;
    this->prePreparePrograms({context->priv().recordTimeAllocator(), writeView, usesMSAASurface,
                             &dstProxyView, renderPassXferBarriers, colorLoadOp,
                             context->priv().caps()},
                             (clip) ? std::move(*clip) : GrAppliedClip::Disabled());
    if (fStencilFanProgram) {
        context->priv().recordProgramInfo(fStencilFanProgram);
    }
    if (fStencilPathProgram) {
        context->priv().recordProgramInfo(fStencilPathProgram);
    }
    if (fCoverBBoxProgram) {
        context->priv().recordProgramInfo(fCoverBBoxProgram);
    }
}

GR_DECLARE_STATIC_UNIQUE_KEY(gUnitQuadBufferKey);

void PathStencilCoverOp::onPrepare(GrOpFlushState* flushState) {
    if (!fTessellator) {
        this->prePreparePrograms({flushState->allocator(), flushState->writeView(),
                                 flushState->usesMSAASurface(), &flushState->dstProxyView(),
                                 flushState->renderPassBarriers(), flushState->colorLoadOp(),
                                 &flushState->caps()}, flushState->detachAppliedClip());
        if (!fTessellator) {
            return;
        }
    }

    if (fStencilFanProgram) {
        // The inner fan isn't built into the tessellator. Generate a standard Redbook fan with a
        // middle-out topology.
        GrEagerDynamicVertexAllocator vertexAlloc(flushState, &fFanBuffer, &fFanBaseVertex);
        int maxCombinedFanEdges = skgpu::tess::PathTessellator::MaxCombinedFanEdgesInPathDrawList(
                fTotalCombinedPathVerbCnt);
        // A single n-sided polygon is fanned by n-2 triangles. Multiple polygons with a combined
        // edge count of n are fanned by strictly fewer triangles.
        int maxTrianglesInFans = std::max(maxCombinedFanEdges - 2, 0);
        GrVertexWriter triangleVertexWriter = vertexAlloc.lock<SkPoint>(maxTrianglesInFans * 3);
        int fanTriangleCount = 0;
        for (auto [pathMatrix, path] : *fPathDrawList) {
            int numTrianglesWritten;
            triangleVertexWriter = skgpu::tess::MiddleOutPolygonTriangulator::WritePathInnerFan(
                    std::move(triangleVertexWriter),
                    0,
                    0,
                    pathMatrix,
                    path,
                    &numTrianglesWritten);
            fanTriangleCount += numTrianglesWritten;
        }
        SkASSERT(fanTriangleCount <= maxTrianglesInFans);
        fFanVertexCount = fanTriangleCount * 3;
        vertexAlloc.unlock(fFanVertexCount);
    }

    fTessellator->prepare(flushState, this->bounds(), *fPathDrawList, fTotalCombinedPathVerbCnt);

    if (fCoverBBoxProgram) {
        size_t instanceStride = fCoverBBoxProgram->geomProc().instanceStride();
        GrVertexWriter vertexWriter = flushState->makeVertexSpace(
                instanceStride,
                fPathCount,
                &fBBoxBuffer,
                &fBBoxBaseInstance);
        SkDEBUGCODE(int pathCount = 0;)
        for (auto [pathMatrix, path] : *fPathDrawList) {
            SkDEBUGCODE(auto end = vertexWriter.makeOffset(instanceStride));
            vertexWriter.write(pathMatrix.getScaleX(),
                               pathMatrix.getSkewY(),
                               pathMatrix.getSkewX(),
                               pathMatrix.getScaleY(),
                               pathMatrix.getTranslateX(),
                               pathMatrix.getTranslateY());
            if (path.isInverseFillType()) {
                // Fill the entire backing store to make sure we clear every stencil value back to
                // 0. If there is a scissor it will have already clipped the stencil draw.
                auto rtBounds =
                        flushState->writeView().asRenderTargetProxy()->backingStoreBoundsRect();
                SkASSERT(rtBounds == fOriginalDrawBounds);
                SkRect pathSpaceRTBounds;
                if (SkMatrixPriv::InverseMapRect(pathMatrix, &pathSpaceRTBounds, rtBounds)) {
                    vertexWriter.write(pathSpaceRTBounds);
                } else {
                    vertexWriter.write(path.getBounds());
                }
            } else {
                vertexWriter.write(path.getBounds());
            }
            SkASSERT(vertexWriter == end);
            SkDEBUGCODE(++pathCount;)
        }
        SkASSERT(pathCount == fPathCount);
    }

    if (!flushState->caps().shaderCaps()->vertexIDSupport()) {
        constexpr static SkPoint kUnitQuad[4] = {{0,0}, {0,1}, {1,0}, {1,1}};

        GR_DEFINE_STATIC_UNIQUE_KEY(gUnitQuadBufferKey);

        fBBoxVertexBufferIfNoIDSupport = flushState->resourceProvider()->findOrMakeStaticBuffer(
                GrGpuBufferType::kVertex, sizeof(kUnitQuad), kUnitQuad, gUnitQuadBufferKey);
    }
}

void PathStencilCoverOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
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
    if (flushState->caps().requiresManualFBBarrierAfterTessellatedStencilDraw()) {
        flushState->gpu()->insertManualFramebufferBarrier();  // http://skbug.com/9739
    }

    // Fill in the bounding box (if not in stencil-only mode).
    if (fCoverBBoxProgram) {
        flushState->bindPipelineAndScissorClip(*fCoverBBoxProgram, this->bounds());
        flushState->bindTextures(fCoverBBoxProgram->geomProc(), nullptr,
                                 fCoverBBoxProgram->pipeline());
        flushState->bindBuffers(nullptr, fBBoxBuffer, fBBoxVertexBufferIfNoIDSupport);
        flushState->drawInstanced(fPathCount, fBBoxBaseInstance, 4, 0);
    }
}

} // namespace skgpu::v1
