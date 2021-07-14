/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrPathStencilCoverOp.h"

#include "src/gpu/GrEagerVertexAllocator.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"
#include "src/gpu/tessellate/GrMiddleOutPolygonTriangulator.h"
#include "src/gpu/tessellate/GrPathCurveTessellator.h"
#include "src/gpu/tessellate/GrPathWedgeTessellator.h"
#include "src/gpu/tessellate/GrTessellationPathRenderer.h"
#include "src/gpu/tessellate/shaders/GrPathTessellationShader.h"

using PathFlags = GrTessellationPathRenderer::PathFlags;

namespace {

// Fills a path's bounding box, with subpixel outset to avoid possible T-junctions with extreme
// edges of the path.
// NOTE: The emitted geometry may not be axis-aligned, depending on the view matrix.
class BoundingBoxShader : public GrPathTessellationShader {
public:
    BoundingBoxShader(const SkMatrix& viewMatrix, SkPMColor4f color, const GrShaderCaps& shaderCaps)
            : GrPathTessellationShader(kTessellate_BoundingBoxShader_ClassID,
                                       GrPrimitiveType::kTriangleStrip, 0, viewMatrix, color) {
        constexpr static Attribute kPathBoundsAttrib("pathBounds", kFloat4_GrVertexAttribType,
                                                     kFloat4_GrSLType);
        this->setInstanceAttributes(&kPathBoundsAttrib, 1);
        if (!shaderCaps.vertexIDSupport()) {
            constexpr static Attribute kUnitCoordAttrib("unitCoord", kFloat2_GrVertexAttribType,
                                                        kFloat2_GrSLType);
            this->setVertexAttributes(&kUnitCoordAttrib, 1);
        }
    }

private:
    const char* name() const final { return "tessellate_BoundingBoxShader"; }
    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const final {}
    GrGLSLGeometryProcessor* createGLSLInstance(const GrShaderCaps&) const final;
};

GrGLSLGeometryProcessor* BoundingBoxShader::createGLSLInstance(const GrShaderCaps&) const {
    class Impl : public GrPathTessellationShader::Impl {
        void emitVertexCode(const GrShaderCaps& shaderCaps, const GrPathTessellationShader&,
                            GrGLSLVertexBuilder* v, GrGPArgs* gpArgs) override {
            if (shaderCaps.vertexIDSupport()) {
                // If we don't have sk_VertexID support then "unitCoord" already came in as a vertex
                // attrib.
                v->codeAppendf(R"(
                float2 unitCoord = float2(sk_VertexID & 1, sk_VertexID >> 1);)");
            }

            v->codeAppend(R"(
            // Bloat the bounding box by 1/4px to avoid potential T-junctions at the edges.
            float2x2 M_ = inverse(AFFINE_MATRIX);
            float2 bloat = float2(abs(M_[0]) + abs(M_[1])) * .25;

            // Find the vertex position.
            float2 localcoord = mix(pathBounds.xy - bloat, pathBounds.zw + bloat, unitCoord);
            float2 vertexpos = AFFINE_MATRIX * localcoord + TRANSLATE;)");
            gpArgs->fLocalCoordVar.set(kFloat2_GrSLType, "localcoord");
            gpArgs->fPositionVar.set(kFloat2_GrSLType, "vertexpos");
        }
    };
    return new Impl;
}

}  // namespace

void GrPathStencilCoverOp::visitProxies(const GrVisitProxyFunc& func) const {
    if (fCoverBBoxProgram) {
        fCoverBBoxProgram->pipeline().visitProxies(func);
    } else {
        fProcessors.visitProxies(func);
    }
}

GrDrawOp::FixedFunctionFlags GrPathStencilCoverOp::fixedFunctionFlags() const {
    auto flags = FixedFunctionFlags::kUsesStencil;
    if (fAAType != GrAAType::kNone) {
        flags |= FixedFunctionFlags::kUsesHWAA;
    }
    return flags;
}

GrProcessorSet::Analysis GrPathStencilCoverOp::finalize(const GrCaps& caps,
                                                        const GrAppliedClip* clip,
                                                        GrClampType clampType) {
    return fProcessors.finalize(fColor, GrProcessorAnalysisCoverage::kNone, clip, nullptr, caps,
                                clampType, &fColor);
}

void GrPathStencilCoverOp::prePreparePrograms(const GrTessellationShader::ProgramArgs& args,
                                              GrAppliedClip&& appliedClip) {
    SkASSERT(!fTessellator);
    SkASSERT(!fStencilFanProgram);
    SkASSERT(!fStencilPathProgram);
    SkASSERT(!fCoverBBoxProgram);

    const GrPipeline* stencilPipeline = GrPathTessellationShader::MakeStencilOnlyPipeline(
            args, fAAType, fPathFlags, appliedClip.hardClip());
    const GrUserStencilSettings* stencilPathSettings =
            GrPathTessellationShader::StencilPathSettings(GrFillRuleForSkPath(fPath));

    if (fPath.countVerbs() > 50 && this->bounds().height() * this->bounds().width() > 256 * 256) {
        // Large complex paths do better with a dedicated triangle shader for the inner fan.
        // This takes less PCI bus bandwidth (6 floats per triangle instead of 8) and allows us
        // to make sure it has an efficient middle-out topology.
        auto shader = GrPathTessellationShader::MakeSimpleTriangleShader(
                args.fArena, fViewMatrix, SK_PMColor4fTRANSPARENT);
        fStencilFanProgram = GrTessellationShader::MakeProgram(args, shader, stencilPipeline,
                                                               stencilPathSettings);
        fTessellator = GrPathCurveTessellator::Make(args.fArena, fViewMatrix,
                                                    SK_PMColor4fTRANSPARENT,
                                                    GrPathCurveTessellator::DrawInnerFan::kNo,
                                                    fPath.countVerbs(), *stencilPipeline,
                                                    *args.fCaps);
    } else {
        fTessellator = GrPathWedgeTessellator::Make(args.fArena, fViewMatrix,
                                                    SK_PMColor4fTRANSPARENT, fPath.countVerbs(),
                                                    *stencilPipeline, *args.fCaps);
    }
    fStencilPathProgram = GrTessellationShader::MakeProgram(args, fTessellator->shader(),
                                                            stencilPipeline, stencilPathSettings);

    if (!(fPathFlags & PathFlags::kStencilOnly)) {
        // Create a program that draws a bounding box over the path and fills its stencil coverage
        // into the color buffer.
        auto* bboxShader = args.fArena->make<BoundingBoxShader>(fViewMatrix, fColor,
                                                                *args.fCaps->shaderCaps());
        auto* bboxPipeline = GrTessellationShader::MakePipeline(args, fAAType,
                                                                std::move(appliedClip),
                                                                std::move(fProcessors));
        auto* bboxStencil =
                GrPathTessellationShader::TestAndResetStencilSettings(fPath.isInverseFillType());
        fCoverBBoxProgram = GrTessellationShader::MakeProgram(args, bboxShader, bboxPipeline,
                                                              bboxStencil);
    }
}

void GrPathStencilCoverOp::onPrePrepare(GrRecordingContext* context,
                                        const GrSurfaceProxyView& writeView, GrAppliedClip* clip,
                                        const GrDstProxyView& dstProxyView,
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
    if (fCoverBBoxProgram) {
        context->priv().recordProgramInfo(fCoverBBoxProgram);
    }
}

GR_DECLARE_STATIC_UNIQUE_KEY(gUnitQuadBufferKey);

void GrPathStencilCoverOp::onPrepare(GrOpFlushState* flushState) {
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

    if (fCoverBBoxProgram) {
        GrVertexWriter vertexWriter = flushState->makeVertexSpace(sizeof(SkRect), 1, &fBBoxBuffer,
                                                                  &fBBoxBaseInstance);
        if (fPath.isInverseFillType()) {
            // Fill the entire backing store to make sure we clear every stencil value back to 0. If
            // there is a scissor it will have already clipped the stencil draw.
            auto rtBounds = flushState->writeView().asRenderTargetProxy()->backingStoreBoundsRect();
            SkASSERT(rtBounds == fOriginalDrawBounds);
            SkRect pathSpaceRTBounds;
            if (SkMatrixPriv::InverseMapRect(fViewMatrix, &pathSpaceRTBounds, rtBounds)) {
                vertexWriter.write(pathSpaceRTBounds);
            } else {
                vertexWriter.write(fPath.getBounds());
            }
        } else {
            vertexWriter.write(fPath.getBounds());
        }
    }

    if (!flushState->caps().shaderCaps()->vertexIDSupport()) {
        constexpr static SkPoint kUnitQuad[4] = {{0,0}, {0,1}, {1,0}, {1,1}};

        GR_DEFINE_STATIC_UNIQUE_KEY(gUnitQuadBufferKey);

        fBBoxVertexBufferIfNoIDSupport = flushState->resourceProvider()->findOrMakeStaticBuffer(
                GrGpuBufferType::kVertex, sizeof(kUnitQuad), kUnitQuad, gUnitQuadBufferKey);
    }
}

void GrPathStencilCoverOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
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
        flushState->drawInstanced(1, fBBoxBaseInstance, 4, 0);
    }
}
