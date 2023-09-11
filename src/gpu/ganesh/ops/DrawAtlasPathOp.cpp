/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/ops/DrawAtlasPathOp.h"

#include "src/gpu/BufferWriter.h"
#include "src/gpu/KeyBuilder.h"
#include "src/gpu/ganesh/GrBuffer.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrGeometryProcessor.h"
#include "src/gpu/ganesh/GrGpuBuffer.h"
#include "src/gpu/ganesh/GrOpFlushState.h"
#include "src/gpu/ganesh/GrOpsRenderPass.h"
#include "src/gpu/ganesh/GrProgramInfo.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrResourceProvider.h"
#include "src/gpu/ganesh/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/ganesh/glsl/GrGLSLVarying.h"
#include "src/gpu/ganesh/glsl/GrGLSLVertexGeoBuilder.h"

using namespace skia_private;

namespace {

class DrawAtlasPathShader : public GrGeometryProcessor {
public:
    DrawAtlasPathShader(bool usesLocalCoords,
                        const skgpu::ganesh::AtlasInstancedHelper* atlasHelper,
                        const GrShaderCaps& shaderCaps)
            : GrGeometryProcessor(kDrawAtlasPathShader_ClassID)
            , fUsesLocalCoords(usesLocalCoords)
            , fAtlasHelper(atlasHelper)
            , fAtlasAccess(GrSamplerState::Filter::kNearest,
                           fAtlasHelper->proxy()->backendFormat(),
                           fAtlasHelper->atlasSwizzle()) {
        if (!shaderCaps.fVertexIDSupport) {
            constexpr static Attribute kUnitCoordAttrib(
                    "unitCoord", kFloat2_GrVertexAttribType, SkSLType::kFloat2);
            this->setVertexAttributesWithImplicitOffsets(&kUnitCoordAttrib, 1);
        }
        fAttribs.emplace_back("fillBounds", kFloat4_GrVertexAttribType, SkSLType::kFloat4);
        if (fUsesLocalCoords) {
            fAttribs.emplace_back("affineMatrix", kFloat4_GrVertexAttribType, SkSLType::kFloat4);
            fAttribs.emplace_back("translate", kFloat2_GrVertexAttribType, SkSLType::kFloat2);
        }
        SkASSERT(fAttribs.size() == this->colorAttribIdx());
        fAttribs.emplace_back("color", kFloat4_GrVertexAttribType, SkSLType::kHalf4);
        fAtlasHelper->appendInstanceAttribs(&fAttribs);
        SkASSERT(fAttribs.size() <= kMaxInstanceAttribs);
        this->setInstanceAttributesWithImplicitOffsets(fAttribs.data(), fAttribs.size());
        this->setTextureSamplerCnt(1);
    }

private:
    class Impl;

    int colorAttribIdx() const { return fUsesLocalCoords ? 3 : 1; }
    const char* name() const override { return "DrawAtlasPathShader"; }
    void addToKey(const GrShaderCaps&, skgpu::KeyBuilder* b) const override {
        b->addBits(1, fUsesLocalCoords, "localCoords");
        fAtlasHelper->getKeyBits(b);
    }
    const TextureSampler& onTextureSampler(int) const override { return fAtlasAccess; }
    std::unique_ptr<ProgramImpl> makeProgramImpl(const GrShaderCaps&) const override;

    const bool fUsesLocalCoords;
    const skgpu::ganesh::AtlasInstancedHelper* const fAtlasHelper;
    TextureSampler fAtlasAccess;
    constexpr static int kMaxInstanceAttribs = 6;
    STArray<kMaxInstanceAttribs, GrGeometryProcessor::Attribute> fAttribs;
};

class DrawAtlasPathShader::Impl : public ProgramImpl {
public:
    void setData(const GrGLSLProgramDataManager& pdman,
                 const GrShaderCaps&,
                 const GrGeometryProcessor& geomProc) override {
        auto* atlasHelper = geomProc.cast<DrawAtlasPathShader>().fAtlasHelper;
        atlasHelper->setUniformData(pdman, fAtlasAdjustUniform);
    }

private:
    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
        const auto& shader = args.fGeomProc.cast<DrawAtlasPathShader>();
        args.fVaryingHandler->emitAttributes(shader);

        if (args.fShaderCaps->fVertexIDSupport) {
            // If we don't have sk_VertexID support then "unitCoord" already came in as a vertex
            // attrib.
            args.fVertBuilder->codeAppendf(R"(
            float2 unitCoord = float2(sk_VertexID & 1, sk_VertexID >> 1);)");
        }

        args.fVertBuilder->codeAppendf(R"(
        float2 devCoord = mix(fillBounds.xy, fillBounds.zw, unitCoord);)");
        gpArgs->fPositionVar.set(SkSLType::kFloat2, "devCoord");

        if (shader.fUsesLocalCoords) {
            args.fVertBuilder->codeAppendf(R"(
            float2x2 M = float2x2(affineMatrix.xy, affineMatrix.zw);
            float2 localCoord = inverse(M) * (devCoord - translate);)");
            gpArgs->fLocalCoordVar.set(SkSLType::kFloat2, "localCoord");
        }

        args.fFragBuilder->codeAppendf("half4 %s = half4(1);", args.fOutputCoverage);
        shader.fAtlasHelper->injectShaderCode(args, gpArgs->fPositionVar, &fAtlasAdjustUniform);

        args.fFragBuilder->codeAppendf("half4 %s;", args.fOutputColor);
        args.fVaryingHandler->addPassThroughAttribute(
                shader.fAttribs[shader.colorAttribIdx()].asShaderVar(),
                args.fOutputColor,
                GrGLSLVaryingHandler::Interpolation::kCanBeFlat);
    }

    GrGLSLUniformHandler::UniformHandle fAtlasAdjustUniform;
};

std::unique_ptr<GrGeometryProcessor::ProgramImpl> DrawAtlasPathShader::makeProgramImpl(
        const GrShaderCaps&) const {
    return std::make_unique<Impl>();
}

}  // anonymous namespace

namespace skgpu::ganesh {

GrProcessorSet::Analysis DrawAtlasPathOp::finalize(const GrCaps& caps, const GrAppliedClip* clip,
                                                   GrClampType clampType) {
    const GrProcessorSet::Analysis& analysis = fProcessors.finalize(
            fHeadInstance->fColor, GrProcessorAnalysisCoverage::kSingleChannel, clip,
            &GrUserStencilSettings::kUnused, caps, clampType, &fHeadInstance->fColor);
    fUsesLocalCoords = analysis.usesLocalCoords();
    return analysis;
}

GrOp::CombineResult DrawAtlasPathOp::onCombineIfPossible(GrOp* op, SkArenaAlloc*, const GrCaps&) {
    auto that = op->cast<DrawAtlasPathOp>();

    if (!fAtlasHelper.isCompatible(that->fAtlasHelper) ||
        fProcessors != that->fProcessors) {
        return CombineResult::kCannotCombine;
    }

    SkASSERT(fUsesLocalCoords == that->fUsesLocalCoords);
    *fTailInstance = that->fHeadInstance;
    fTailInstance = that->fTailInstance;
    fInstanceCount += that->fInstanceCount;
    return CombineResult::kMerged;
}

void DrawAtlasPathOp::prepareProgram(const GrCaps& caps, SkArenaAlloc* arena,
                                     const GrSurfaceProxyView& writeView, bool usesMSAASurface,
                                     GrAppliedClip&& appliedClip,
                                     const GrDstProxyView& dstProxyView,
                                     GrXferBarrierFlags renderPassXferBarriers,
                                     GrLoadOp colorLoadOp) {
    SkASSERT(!fProgram);
    GrPipeline::InitArgs initArgs;
    initArgs.fCaps = &caps;
    initArgs.fDstProxyView = dstProxyView;
    initArgs.fWriteSwizzle = writeView.swizzle();
    auto pipeline = arena->make<GrPipeline>(initArgs, std::move(fProcessors),
                                            std::move(appliedClip));
    auto shader = arena->make<DrawAtlasPathShader>(fUsesLocalCoords, &fAtlasHelper,
                                                   *caps.shaderCaps());
    fProgram = arena->make<GrProgramInfo>(caps, writeView, usesMSAASurface, pipeline,
                                          &GrUserStencilSettings::kUnused, shader,
                                          GrPrimitiveType::kTriangleStrip,
                                          renderPassXferBarriers, colorLoadOp);
}

void DrawAtlasPathOp::onPrePrepare(GrRecordingContext* rContext,
                                   const GrSurfaceProxyView& writeView,
                                   GrAppliedClip* appliedClip, const GrDstProxyView& dstProxyView,
                                   GrXferBarrierFlags renderPassXferBarriers,
                                   GrLoadOp colorLoadOp) {
    // DMSAA is not supported on DDL.
    bool usesMSAASurface = writeView.asRenderTargetProxy()->numSamples() > 1;
    this->prepareProgram(*rContext->priv().caps(), rContext->priv().recordTimeAllocator(),
                         writeView, usesMSAASurface, std::move(*appliedClip), dstProxyView,
                         renderPassXferBarriers, colorLoadOp);
    SkASSERT(fProgram);
    rContext->priv().recordProgramInfo(fProgram);
}

SKGPU_DECLARE_STATIC_UNIQUE_KEY(gUnitQuadBufferKey);

void DrawAtlasPathOp::onPrepare(GrOpFlushState* flushState) {
    if (!fProgram) {
        this->prepareProgram(flushState->caps(), flushState->allocator(), flushState->writeView(),
                             flushState->usesMSAASurface(), flushState->detachAppliedClip(),
                             flushState->dstProxyView(), flushState->renderPassBarriers(),
                             flushState->colorLoadOp());
        SkASSERT(fProgram);
    }

    if (VertexWriter instanceWriter = flushState->makeVertexWriter(
                fProgram->geomProc().instanceStride(), fInstanceCount, &fInstanceBuffer,
                &fBaseInstance)) {
        for (const Instance* i = fHeadInstance; i; i = i->fNext) {
            instanceWriter << SkRect::Make(i->fFillBounds)
                           << VertexWriter::If(fUsesLocalCoords,
                                               i->fLocalToDeviceIfUsingLocalCoords)
                           << i->fColor;
            fAtlasHelper.writeInstanceData(&instanceWriter, &i->fAtlasInstance);
        }
    }

    if (!flushState->caps().shaderCaps()->fVertexIDSupport) {
        constexpr static SkPoint kUnitQuad[4] = {{0,0}, {0,1}, {1,0}, {1,1}};

        SKGPU_DEFINE_STATIC_UNIQUE_KEY(gUnitQuadBufferKey);

        fVertexBufferIfNoIDSupport = flushState->resourceProvider()->findOrMakeStaticBuffer(
                GrGpuBufferType::kVertex, sizeof(kUnitQuad), kUnitQuad, gUnitQuadBufferKey);
    }
}

void DrawAtlasPathOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
    if (fProgram->geomProc().hasVertexAttributes() && !fVertexBufferIfNoIDSupport) {
        return;
    }
    flushState->bindPipelineAndScissorClip(*fProgram, this->bounds());
    flushState->bindTextures(fProgram->geomProc(), *fAtlasHelper.proxy(), fProgram->pipeline());
    flushState->bindBuffers(nullptr, std::move(fInstanceBuffer), fVertexBufferIfNoIDSupport);
    flushState->drawInstanced(fInstanceCount, fBaseInstance, 4, 0);
}

}  // namespace skgpu::ganesh
