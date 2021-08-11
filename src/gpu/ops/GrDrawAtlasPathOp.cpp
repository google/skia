/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/GrDrawAtlasPathOp.h"

#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrOpsRenderPass.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrVertexWriter.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"

namespace {

class DrawAtlasPathShader : public GrGeometryProcessor {
public:
    DrawAtlasPathShader(bool usesLocalCoords,
                        const GrAtlasInstancedHelper* atlasHelper,
                        const GrShaderCaps& shaderCaps)
            : GrGeometryProcessor(kDrawAtlasPathShader_ClassID)
            , fUsesLocalCoords(usesLocalCoords)
            , fAtlasHelper(atlasHelper)
            , fAtlasAccess(GrSamplerState::Filter::kNearest,
                           fAtlasHelper->proxy()->backendFormat(),
                           fAtlasHelper->atlasSwizzle()) {
        if (!shaderCaps.vertexIDSupport()) {
            constexpr static Attribute kUnitCoordAttrib(
                    "unitCoord", kFloat2_GrVertexAttribType, kFloat2_GrSLType);
            this->setVertexAttributes(&kUnitCoordAttrib, 1);
        }
        fAttribs.emplace_back("fillBounds", kFloat4_GrVertexAttribType, kFloat4_GrSLType);
        if (fUsesLocalCoords) {
            fAttribs.emplace_back("affineMatrix", kFloat4_GrVertexAttribType, kFloat4_GrSLType);
            fAttribs.emplace_back("translate", kFloat2_GrVertexAttribType, kFloat2_GrSLType);
        }
        SkASSERT(fAttribs.count() == this->colorAttribIdx());
        fAttribs.emplace_back("color", kFloat4_GrVertexAttribType, kHalf4_GrSLType);
        fAtlasHelper->appendInstanceAttribs(&fAttribs);
        SkASSERT(fAttribs.count() <= kMaxInstanceAttribs);
        this->setInstanceAttributes(fAttribs.data(), fAttribs.count());
        this->setTextureSamplerCnt(1);
    }

private:
    class Impl;

    int colorAttribIdx() const { return fUsesLocalCoords ? 3 : 1; }
    const char* name() const override { return "DrawAtlasPathShader"; }
    void addToKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const override {
        b->addBits(1, fUsesLocalCoords, "localCoords");
        fAtlasHelper->getKeyBits(b);
    }
    const TextureSampler& onTextureSampler(int) const override { return fAtlasAccess; }
    std::unique_ptr<ProgramImpl> makeProgramImpl(const GrShaderCaps&) const override;

    const bool fUsesLocalCoords;
    const GrAtlasInstancedHelper* const fAtlasHelper;
    TextureSampler fAtlasAccess;
    constexpr static int kMaxInstanceAttribs = 6;
    SkSTArray<kMaxInstanceAttribs, GrGeometryProcessor::Attribute> fAttribs;
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

        if (args.fShaderCaps->vertexIDSupport()) {
            // If we don't have sk_VertexID support then "unitCoord" already came in as a vertex
            // attrib.
            args.fVertBuilder->codeAppendf(R"(
            float2 unitCoord = float2(sk_VertexID & 1, sk_VertexID >> 1);)");
        }

        args.fVertBuilder->codeAppendf(R"(
        float2 devCoord = mix(fillBounds.xy, fillBounds.zw, unitCoord);)");
        gpArgs->fPositionVar.set(kFloat2_GrSLType, "devCoord");

        if (shader.fUsesLocalCoords) {
            args.fVertBuilder->codeAppendf(R"(
            float2x2 M = float2x2(affineMatrix);
            float2 localCoord = inverse(M) * (devCoord - translate);)");
            gpArgs->fLocalCoordVar.set(kFloat2_GrSLType, "localCoord");
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

}  // namespace

GrProcessorSet::Analysis GrDrawAtlasPathOp::finalize(const GrCaps& caps, const GrAppliedClip* clip,
                                                     GrClampType clampType) {
    const GrProcessorSet::Analysis& analysis = fProcessors.finalize(
            fHeadInstance->fColor, GrProcessorAnalysisCoverage::kSingleChannel, clip,
            &GrUserStencilSettings::kUnused, caps, clampType, &fHeadInstance->fColor);
    fUsesLocalCoords = analysis.usesLocalCoords();
    return analysis;
}

GrOp::CombineResult GrDrawAtlasPathOp::onCombineIfPossible(GrOp* op, SkArenaAlloc*, const GrCaps&) {
    auto* that = op->cast<GrDrawAtlasPathOp>();

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

void GrDrawAtlasPathOp::prepareProgram(const GrCaps& caps, SkArenaAlloc* arena,
                                       const GrSurfaceProxyView& writeView, bool usesMSAASurface,
                                       GrAppliedClip&& appliedClip,
                                       const GrDstProxyView& dstProxyView,
                                       GrXferBarrierFlags renderPassXferBarriers,
                                       GrLoadOp colorLoadOp) {
    SkASSERT(!fProgram);
    GrPipeline::InitArgs initArgs;
    if (usesMSAASurface) {
        initArgs.fInputFlags |= GrPipeline::InputFlags::kHWAntialias;
    }
    initArgs.fCaps = &caps;
    initArgs.fDstProxyView = dstProxyView;
    initArgs.fWriteSwizzle = writeView.swizzle();
    auto pipeline = arena->make<GrPipeline>(initArgs, std::move(fProcessors),
                                            std::move(appliedClip));
    auto shader = arena->make<DrawAtlasPathShader>(fUsesLocalCoords, &fAtlasHelper,
                                                   *caps.shaderCaps());
    fProgram = arena->make<GrProgramInfo>(writeView, pipeline, &GrUserStencilSettings::kUnused,
                                          shader, GrPrimitiveType::kTriangleStrip, 0,
                                          renderPassXferBarriers, colorLoadOp);
}

void GrDrawAtlasPathOp::onPrePrepare(GrRecordingContext* rContext,
                                     const GrSurfaceProxyView& writeView,
                                     GrAppliedClip* appliedClip, const GrDstProxyView& dstProxyView,
                                     GrXferBarrierFlags renderPassXferBarriers,
                                     GrLoadOp colorLoadOp) {
    this->prepareProgram(*rContext->priv().caps(), rContext->priv().recordTimeAllocator(),
                         writeView, writeView.asRenderTargetProxy()->numSamples() > 1,
                         std::move(*appliedClip), dstProxyView, renderPassXferBarriers,
                         colorLoadOp);
    SkASSERT(fProgram);
    rContext->priv().recordProgramInfo(fProgram);
}

GR_DECLARE_STATIC_UNIQUE_KEY(gUnitQuadBufferKey);

void GrDrawAtlasPathOp::onPrepare(GrOpFlushState* flushState) {
    if (!fProgram) {
        this->prepareProgram(flushState->caps(), flushState->allocator(), flushState->writeView(),
                             flushState->usesMSAASurface(), flushState->detachAppliedClip(),
                             flushState->dstProxyView(), flushState->renderPassBarriers(),
                             flushState->colorLoadOp());
        SkASSERT(fProgram);
    }

    // FIXME(skbug.com/12201): Our draw's MSAA state should match the render target, but DDL doesn't
    // yet communicate DMSAA state to onPrePrepare.
    SkASSERT(fProgram->pipeline().isHWAntialiasState() == flushState->usesMSAASurface());

    if (GrVertexWriter instanceWriter = flushState->makeVertexSpace(
                fProgram->geomProc().instanceStride(), fInstanceCount, &fInstanceBuffer,
                &fBaseInstance)) {
        for (const Instance* i = fHeadInstance; i; i = i->fNext) {
            instanceWriter.write(
                    SkRect::Make(i->fFillBounds),
                    GrVertexWriter::If(fUsesLocalCoords,
                                       i->fLocalToDeviceIfUsingLocalCoords),
                    i->fColor);
            fAtlasHelper.writeInstanceData(&instanceWriter, &i->fAtlasInstance);
        }
    }

    if (!flushState->caps().shaderCaps()->vertexIDSupport()) {
        constexpr static SkPoint kUnitQuad[4] = {{0,0}, {0,1}, {1,0}, {1,1}};

        GR_DEFINE_STATIC_UNIQUE_KEY(gUnitQuadBufferKey);

        fVertexBufferIfNoIDSupport = flushState->resourceProvider()->findOrMakeStaticBuffer(
                GrGpuBufferType::kVertex, sizeof(kUnitQuad), kUnitQuad, gUnitQuadBufferKey);
    }
}

void GrDrawAtlasPathOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
    flushState->bindPipelineAndScissorClip(*fProgram, this->bounds());
    flushState->bindTextures(fProgram->geomProc(), *fAtlasHelper.proxy(), fProgram->pipeline());
    flushState->bindBuffers(nullptr, std::move(fInstanceBuffer), fVertexBufferIfNoIDSupport);
    flushState->drawInstanced(fInstanceCount, fBaseInstance, 4, 0);
}
