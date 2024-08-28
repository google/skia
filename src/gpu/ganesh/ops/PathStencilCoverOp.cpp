/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/ops/PathStencilCoverOp.h"

#include "include/core/SkMatrix.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkPoint.h"
#include "include/gpu/ganesh/GrRecordingContext.h"
#include "include/private/base/SkAlignedStorage.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkOnce.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/ResourceKey.h"
#include "src/gpu/ganesh/GrAppliedClip.h"
#include "src/gpu/ganesh/GrEagerVertexAllocator.h"
#include "src/gpu/ganesh/GrGeometryProcessor.h"
#include "src/gpu/ganesh/GrOpFlushState.h"
#include "src/gpu/ganesh/GrPipeline.h"
#include "src/gpu/ganesh/GrProcessorAnalysis.h"
#include "src/gpu/ganesh/GrProgramInfo.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrRenderTargetProxy.h"
#include "src/gpu/ganesh/GrResourceProvider.h"
#include "src/gpu/ganesh/GrShaderCaps.h"
#include "src/gpu/ganesh/GrShaderVar.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/ganesh/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/ganesh/glsl/GrGLSLUniformHandler.h"
#include "src/gpu/ganesh/glsl/GrGLSLVarying.h"
#include "src/gpu/ganesh/glsl/GrGLSLVertexGeoBuilder.h"
#include "src/gpu/ganesh/ops/FillPathFlags.h"
#include "src/gpu/ganesh/ops/GrSimpleMeshDrawOpHelper.h"
#include "src/gpu/ganesh/tessellate/GrPathTessellationShader.h"
#include "src/gpu/tessellate/AffineMatrix.h"
#include "src/gpu/tessellate/MiddleOutPolygonTriangulator.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <memory>

class GrDstProxyView;
enum class GrXferBarrierFlags;
namespace skgpu {
class KeyBuilder;
}
struct GrUserStencilSettings;

namespace {

// Fills a path's bounding box, with subpixel outset to avoid possible T-junctions with extreme
// edges of the path.
// NOTE: The emitted geometry may not be axis-aligned, depending on the view matrix.
class BoundingBoxShader : public GrGeometryProcessor {
public:
    BoundingBoxShader(SkPMColor4f color, const GrShaderCaps& shaderCaps)
            : GrGeometryProcessor(kTessellate_BoundingBoxShader_ClassID)
            , fColor(color) {
        if (!shaderCaps.fVertexIDSupport) {
            constexpr static Attribute kUnitCoordAttrib("unitCoord", kFloat2_GrVertexAttribType,
                                                        SkSLType::kFloat2);
            this->setVertexAttributesWithImplicitOffsets(&kUnitCoordAttrib, 1);
        }
        constexpr static Attribute kInstanceAttribs[] = {
            {"matrix2d", kFloat4_GrVertexAttribType, SkSLType::kFloat4},
            {"translate", kFloat2_GrVertexAttribType, SkSLType::kFloat2},
            {"pathBounds", kFloat4_GrVertexAttribType, SkSLType::kFloat4}
        };
        this->setInstanceAttributesWithImplicitOffsets(kInstanceAttribs,
                                                       std::size(kInstanceAttribs));
    }

private:
    const char* name() const final { return "tessellate_BoundingBoxShader"; }
    void addToKey(const GrShaderCaps&, skgpu::KeyBuilder*) const final {}
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
            if (args.fShaderCaps->fVertexIDSupport) {
                // If we don't have sk_VertexID support then "unitCoord" already came in as a vertex
                // attrib.
                args.fVertBuilder->codeAppend(
                "float2 unitCoord = float2(sk_VertexID & 1, sk_VertexID >> 1);");
            }
            args.fVertBuilder->codeAppend(
            // Bloat the bounding box by 1/4px to be certain we will reset every stencil value.
            "float2x2 M_ = inverse(float2x2(matrix2d.xy, matrix2d.zw));"
            "float2 bloat = float2(abs(M_[0]) + abs(M_[1])) * .25;"

            // Find the vertex position.
            "float2 localcoord = mix(pathBounds.xy - bloat, pathBounds.zw + bloat, unitCoord);"
            "float2 vertexpos = float2x2(matrix2d.xy, matrix2d.zw) * localcoord + translate;"
            );
            gpArgs->fLocalCoordVar.set(SkSLType::kFloat2, "localcoord");
            gpArgs->fPositionVar.set(SkSLType::kFloat2, "vertexpos");

            // Fragment shader.
            const char* color;
            fColorUniform = args.fUniformHandler->addUniform(nullptr, kFragment_GrShaderFlag,
                                                             SkSLType::kHalf4, "color", &color);
            args.fFragBuilder->codeAppendf("half4 %s = %s;", args.fOutputColor, color);
            args.fFragBuilder->codeAppendf("const half4 %s = half4(1);", args.fOutputCoverage);
        }

        GrGLSLUniformHandler::UniformHandle fColorUniform;
    };

    return std::make_unique<Impl>();
}

}  // anonymous namespace

namespace skgpu::ganesh {

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
        fTessellator = PathCurveTessellator::Make(args.fArena,
                                                  args.fCaps->shaderCaps()->fInfinitySupport);
    } else {
        fTessellator = PathWedgeTessellator::Make(args.fArena,
                                                  args.fCaps->shaderCaps()->fInfinitySupport);
    }
    auto* tessShader = GrPathTessellationShader::Make(*args.fCaps->shaderCaps(),
                                                      args.fArena,
                                                      shaderMatrix,
                                                      SK_PMColor4fTRANSPARENT,
                                                      fTessellator->patchAttribs());
    fStencilPathProgram = GrTessellationShader::MakeProgram(args,
                                                            tessShader,
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

SKGPU_DECLARE_STATIC_UNIQUE_KEY(gUnitQuadBufferKey);

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
        // Path fans might have an extra edge from an implicit kClose at the end, but they also
        // always begin with kMove. So the max possible number of edges in a single path is equal to
        // the number of verbs. Therefore, the max number of combined fan edges in a path list is
        // the number of combined verbs from the paths in the list.
        // A single n-sided polygon is fanned by n-2 triangles. Multiple polygons with a combined
        // edge count of n are fanned by strictly fewer triangles.
        int maxTrianglesInFans = std::max(fTotalCombinedPathVerbCnt - 2, 0);
        int fanTriangleCount = 0;
        if (VertexWriter triangleVertexWriter =
                    vertexAlloc.lockWriter(sizeof(SkPoint), maxTrianglesInFans * 3)) {
            for (auto [pathMatrix, path, color] : *fPathDrawList) {
                tess::AffineMatrix m(pathMatrix);
                for (tess::PathMiddleOutFanIter it(path); !it.done();) {
                    for (auto [p0, p1, p2] : it.nextStack()) {
                        triangleVertexWriter << m.map2Points(p0, p1) << m.mapPoint(p2);
                        ++fanTriangleCount;
                    }
                }
            }


            SkASSERT(fanTriangleCount <= maxTrianglesInFans);
            fFanVertexCount = fanTriangleCount * 3;
            vertexAlloc.unlock(fFanVertexCount);
        }
    }

    auto tessShader = &fStencilPathProgram->geomProc().cast<GrPathTessellationShader>();
    fTessellator->prepare(flushState,
                          tessShader->viewMatrix(),
                          *fPathDrawList,
                          fTotalCombinedPathVerbCnt);

    if (fCoverBBoxProgram) {
        size_t instanceStride = fCoverBBoxProgram->geomProc().instanceStride();
        VertexWriter vertexWriter = flushState->makeVertexWriter(instanceStride,
                                                                 fPathCount,
                                                                 &fBBoxBuffer,
                                                                 &fBBoxBaseInstance);
        SkDEBUGCODE(int pathCount = 0;)
        for (auto [pathMatrix, path, color] : *fPathDrawList) {
            SkDEBUGCODE(auto end = vertexWriter.mark(instanceStride));
            vertexWriter << pathMatrix.getScaleX()
                         << pathMatrix.getSkewY()
                         << pathMatrix.getSkewX()
                         << pathMatrix.getScaleY()
                         << pathMatrix.getTranslateX()
                         << pathMatrix.getTranslateY();
            if (path.isInverseFillType()) {
                // Fill the entire backing store to make sure we clear every stencil value back to
                // 0. If there is a scissor it will have already clipped the stencil draw.
                auto rtBounds =
                        flushState->writeView().asRenderTargetProxy()->backingStoreBoundsRect();
                SkASSERT(rtBounds == fOriginalDrawBounds);
                SkRect pathSpaceRTBounds;
                if (SkMatrixPriv::InverseMapRect(pathMatrix, &pathSpaceRTBounds, rtBounds)) {
                    vertexWriter << pathSpaceRTBounds;
                } else {
                    vertexWriter << path.getBounds();
                }
            } else {
                vertexWriter << path.getBounds();
            }
            SkASSERT(vertexWriter.mark() == end);
            SkDEBUGCODE(++pathCount;)
        }
        SkASSERT(pathCount == fPathCount);
    }

    if (!flushState->caps().shaderCaps()->fVertexIDSupport) {
        constexpr static SkPoint kUnitQuad[4] = {{0,0}, {0,1}, {1,0}, {1,1}};

        SKGPU_DEFINE_STATIC_UNIQUE_KEY(gUnitQuadBufferKey);

        fBBoxVertexBufferIfNoIDSupport = flushState->resourceProvider()->findOrMakeStaticBuffer(
                GrGpuBufferType::kVertex, sizeof(kUnitQuad), kUnitQuad, gUnitQuadBufferKey);
    }
}

void PathStencilCoverOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
    if (!fTessellator) {
        return;
    }

    if (fCoverBBoxProgram &&
        fCoverBBoxProgram->geomProc().hasVertexAttributes() &&
        !fBBoxVertexBufferIfNoIDSupport) {
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
    if (fCoverBBoxProgram) {
        flushState->bindPipelineAndScissorClip(*fCoverBBoxProgram, this->bounds());
        flushState->bindTextures(fCoverBBoxProgram->geomProc(), nullptr,
                                 fCoverBBoxProgram->pipeline());
        flushState->bindBuffers(nullptr, fBBoxBuffer, fBBoxVertexBufferIfNoIDSupport);
        flushState->drawInstanced(fPathCount, fBBoxBaseInstance, 4, 0);
    }
}

}  // namespace skgpu::ganesh
