/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrDrawAtlasPathOp.h"

#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrOpsRenderPass.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrVertexWriter.h"
#include "src/gpu/GrVx.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"

namespace {

class DrawAtlasPathShader : public GrGeometryProcessor {
public:
    DrawAtlasPathShader(const GrTextureProxy* atlasProxy, GrSwizzle swizzle, bool isInverseFill,
                        bool usesLocalCoords, const GrShaderCaps& shaderCaps)
            : GrGeometryProcessor(kDrawAtlasPathShader_ClassID)
            , fAtlasAccess(GrSamplerState::Filter::kNearest, atlasProxy->backendFormat(), swizzle)
            , fAtlasDimensions(atlasProxy->backingStoreDimensions())
            , fIsInverseFill(isInverseFill)
            , fUsesLocalCoords(usesLocalCoords) {
        if (!shaderCaps.vertexIDSupport()) {
            constexpr static Attribute kUnitCoordAttrib("unitCoord", kFloat2_GrVertexAttribType,
                                                        kFloat2_GrSLType);
            this->setVertexAttributes(&kUnitCoordAttrib, 1);
        }
        fAttribs.emplace_back("fillBounds", kFloat4_GrVertexAttribType, kFloat4_GrSLType);
        if (fUsesLocalCoords) {
            fAttribs.emplace_back("affineMatrix", kFloat4_GrVertexAttribType, kFloat4_GrSLType);
            fAttribs.emplace_back("translate", kFloat2_GrVertexAttribType, kFloat2_GrSLType);
        }
        SkASSERT(fAttribs.count() == this->colorAttribIdx());
        fAttribs.emplace_back("color", kFloat4_GrVertexAttribType, kHalf4_GrSLType);
        fAttribs.emplace_back("locations", kFloat4_GrVertexAttribType, kFloat4_GrSLType);
        if (fIsInverseFill) {
            fAttribs.emplace_back("sizeInAtlas", kFloat2_GrVertexAttribType, kFloat2_GrSLType);
        }
        this->setInstanceAttributes(fAttribs.data(), fAttribs.count());
        this->setTextureSamplerCnt(1);
    }

private:
    int colorAttribIdx() const { return fUsesLocalCoords ? 3 : 1; }
    const char* name() const override { return "DrawAtlasPathShader"; }
    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const override {
        b->add32((fUsesLocalCoords << 1) | (int)fIsInverseFill);
    }
    const TextureSampler& onTextureSampler(int) const override { return fAtlasAccess; }
    GrGLSLGeometryProcessor* createGLSLInstance(const GrShaderCaps&) const override;

    const TextureSampler fAtlasAccess;
    const SkISize fAtlasDimensions;
    const bool fIsInverseFill;
    const bool fUsesLocalCoords;
    SkSTArray<6, GrGeometryProcessor::Attribute> fAttribs;

    class Impl;
};

class DrawAtlasPathShader::Impl : public GrGLSLGeometryProcessor {
    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
        const auto& shader = args.fGeomProc.cast<DrawAtlasPathShader>();
        args.fVaryingHandler->emitAttributes(shader);

        GrGLSLVarying atlasCoord(kFloat2_GrSLType);
        args.fVaryingHandler->addVarying("atlasCoord", &atlasCoord);

        const char* atlasAdjust;
        fAtlasAdjustUniform = args.fUniformHandler->addUniform(
                nullptr, kVertex_GrShaderFlag, kFloat2_GrSLType, "atlas_adjust", &atlasAdjust);

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

        args.fVertBuilder->codeAppendf(R"(
        // A negative x coordinate in the atlas indicates that the path is transposed.
        // We also added 1 since we can't negate zero.
        float2 atlasTopLeft = float2(abs(locations.x) - 1, locations.y);
        float2 devTopLeft = locations.zw;
        bool transposed = locations.x < 0;
        float2 atlasCoord = devCoord - devTopLeft;
        if (transposed) {
            atlasCoord = atlasCoord.yx;
        }
        atlasCoord += atlasTopLeft;
        %s = atlasCoord * %s;)", atlasCoord.vsOut(), atlasAdjust);

        if (shader.fIsInverseFill) {
            GrGLSLVarying atlasBounds(kFloat4_GrSLType);
            args.fVaryingHandler->addVarying("atlasbounds", &atlasBounds,
                                             GrGLSLVaryingHandler::Interpolation::kCanBeFlat);
            args.fVertBuilder->codeAppendf(R"(
            float4 atlasBounds = atlasTopLeft.xyxy + (transposed ? sizeInAtlas.00yx
                                                                 : sizeInAtlas.00xy);
            %s = atlasBounds * %s.xyxy;)", atlasBounds.vsOut(), atlasAdjust);

            args.fFragBuilder->codeAppendf(R"(
            half coverage = 0;
            float2 atlasCoord = %s;
            float4 atlasBounds = %s;
            if (all(greaterThan(atlasCoord, atlasBounds.xy)) &&
                all(lessThan(atlasCoord, atlasBounds.zw))) {
                coverage = )", atlasCoord.fsIn(), atlasBounds.fsIn());
            args.fFragBuilder->appendTextureLookup(args.fTexSamplers[0], "atlasCoord");
            args.fFragBuilder->codeAppendf(R"(.a;
            }
            half4 %s = half4(1 - coverage);)", args.fOutputCoverage);
        } else {
            args.fFragBuilder->codeAppendf("half4 %s = ", args.fOutputCoverage);
            args.fFragBuilder->appendTextureLookup(args.fTexSamplers[0], atlasCoord.fsIn());
            args.fFragBuilder->codeAppendf(".aaaa;");
        }

        args.fFragBuilder->codeAppendf("half4 %s;", args.fOutputColor);
        args.fVaryingHandler->addPassThroughAttribute(
                shader.fAttribs[shader.colorAttribIdx()], args.fOutputColor,
                GrGLSLVaryingHandler::Interpolation::kCanBeFlat);
    }

    void setData(const GrGLSLProgramDataManager& pdman,
                 const GrShaderCaps&,
                 const GrGeometryProcessor& geomProc) override {
        const SkISize& dimensions = geomProc.cast<DrawAtlasPathShader>().fAtlasDimensions;
        pdman.set2f(fAtlasAdjustUniform, 1.f / dimensions.width(), 1.f / dimensions.height());
    }

    GrGLSLUniformHandler::UniformHandle fAtlasAdjustUniform;
};

GrGLSLGeometryProcessor* DrawAtlasPathShader::createGLSLInstance(const GrShaderCaps&) const {
    return new Impl();
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

GrOp::CombineResult GrDrawAtlasPathOp::onCombineIfPossible(
        GrOp* op, SkArenaAlloc* alloc, const GrCaps&) {
    auto* that = op->cast<GrDrawAtlasPathOp>();
    SkASSERT(fAtlasProxy == that->fAtlasProxy);
    SkASSERT(fEnableHWAA == that->fEnableHWAA);

    if (fIsInverseFill != that->fIsInverseFill || fProcessors != that->fProcessors) {
        return CombineResult::kCannotCombine;
    }

    SkASSERT(fUsesLocalCoords == that->fUsesLocalCoords);
    *fTailInstance = that->fHeadInstance;
    fTailInstance = that->fTailInstance;
    fInstanceCount += that->fInstanceCount;
    return CombineResult::kMerged;
}

void GrDrawAtlasPathOp::prepareProgram(const GrCaps& caps, SkArenaAlloc* arena,
                                       const GrSurfaceProxyView& writeView,
                                       GrAppliedClip&& appliedClip,
                                       const GrDstProxyView& dstProxyView,
                                       GrXferBarrierFlags renderPassXferBarriers,
                                       GrLoadOp colorLoadOp) {
    SkASSERT(!fProgram);
    GrPipeline::InitArgs initArgs;
    if (fEnableHWAA) {
        initArgs.fInputFlags |= GrPipeline::InputFlags::kHWAntialias;
    }
    initArgs.fCaps = &caps;
    initArgs.fDstProxyView = dstProxyView;
    initArgs.fWriteSwizzle = writeView.swizzle();
    auto pipeline = arena->make<GrPipeline>(initArgs, std::move(fProcessors),
                                            std::move(appliedClip));
    GrSwizzle swizzle = caps.getReadSwizzle(fAtlasProxy->backendFormat(), GrColorType::kAlpha_8);
    auto shader = arena->make<DrawAtlasPathShader>(fAtlasProxy.get(), swizzle, fIsInverseFill,
                                                   fUsesLocalCoords, *caps.shaderCaps());
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
                         writeView, std::move(*appliedClip), dstProxyView, renderPassXferBarriers,
                         colorLoadOp);
    SkASSERT(fProgram);
    rContext->priv().recordProgramInfo(fProgram);
}

GR_DECLARE_STATIC_UNIQUE_KEY(gUnitQuadBufferKey);

void GrDrawAtlasPathOp::onPrepare(GrOpFlushState* flushState) {
    if (!fProgram) {
        this->prepareProgram(flushState->caps(), flushState->allocator(), flushState->writeView(),
                             flushState->detachAppliedClip(), flushState->dstProxyView(),
                             flushState->renderPassBarriers(), flushState->colorLoadOp());
        SkASSERT(fProgram);
    }

    if (GrVertexWriter instanceWriter = flushState->makeVertexSpace(
                fProgram->geomProc().instanceStride(), fInstanceCount, &fInstanceBuffer,
                &fBaseInstance)) {
        for (const Instance* instance = fHeadInstance; instance; instance = instance->fNext) {
            SkASSERT(instance->fLocationInAtlas.x() >= 0);
            SkASSERT(instance->fLocationInAtlas.y() >= 0);
            instanceWriter.write(
                    SkRect::Make(instance->fFillBounds),
                    GrVertexWriter::If(fUsesLocalCoords,
                                       instance->fLocalToDeviceIfUsingLocalCoords),
                    instance->fColor,
                    // A negative x coordinate in the atlas indicates that the path is transposed.
                    // Also add 1 since we can't negate zero.
                    (float)(instance->fTransposedInAtlas ? -instance->fLocationInAtlas.x() - 1
                                                         : instance->fLocationInAtlas.x() + 1),
                    (float)instance->fLocationInAtlas.y(),
                    (float)instance->fPathDevIBounds.left(),
                    (float)instance->fPathDevIBounds.top(),
                    GrVertexWriter::If(fIsInverseFill,
                                       SkSize::Make(instance->fPathDevIBounds.size())));
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
    SkASSERT(fAtlasProxy->isInstantiated());
    flushState->bindPipelineAndScissorClip(*fProgram, this->bounds());
    flushState->bindTextures(fProgram->geomProc(), *fAtlasProxy, fProgram->pipeline());
    flushState->bindBuffers(nullptr, std::move(fInstanceBuffer), fVertexBufferIfNoIDSupport);
    flushState->drawInstanced(fInstanceCount, fBaseInstance, 4, 0);
}
