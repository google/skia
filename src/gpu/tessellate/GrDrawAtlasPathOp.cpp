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
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"

namespace {

constexpr static GrGeometryProcessor::Attribute kInstanceAttribs[] = {
        {"devibounds", kInt4_GrVertexAttribType, kInt4_GrSLType},
        {"dev_to_atlas_offset", kInt2_GrVertexAttribType, kInt2_GrSLType},
        {"color", kFloat4_GrVertexAttribType, kHalf4_GrSLType},
        {"viewmatrix_scaleskew", kFloat4_GrVertexAttribType, kFloat4_GrSLType},
        {"viewmatrix_trans", kFloat2_GrVertexAttribType, kFloat2_GrSLType}};

class DrawAtlasPathShader : public GrGeometryProcessor {
public:
    DrawAtlasPathShader(const GrTextureProxy* atlasProxy, GrSwizzle swizzle, bool usesLocalCoords)
            : GrGeometryProcessor(kDrawAtlasPathShader_ClassID)
            , fAtlasAccess(GrSamplerState::Filter::kNearest, atlasProxy->backendFormat(), swizzle)
            , fAtlasDimensions(atlasProxy->backingStoreDimensions())
            , fUsesLocalCoords(usesLocalCoords) {
        int numInstanceAttribs = SK_ARRAY_COUNT(kInstanceAttribs);
        if (!fUsesLocalCoords) {
            numInstanceAttribs -= 2;
        }
        this->setInstanceAttributes(kInstanceAttribs, numInstanceAttribs);
        this->setTextureSamplerCnt(1);
    }

private:
    const char* name() const override { return "DrawAtlasPathShader"; }
    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const override {
        b->add32(fUsesLocalCoords);
    }
    const TextureSampler& onTextureSampler(int) const override { return fAtlasAccess; }
    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const override;

    const TextureSampler fAtlasAccess;
    const SkISize fAtlasDimensions;
    const bool fUsesLocalCoords;

    class Impl;
};

class DrawAtlasPathShader::Impl : public GrGLSLGeometryProcessor {
    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
        const auto& shader = args.fGP.cast<DrawAtlasPathShader>();
        args.fVaryingHandler->emitAttributes(shader);

        GrGLSLVarying atlasCoord(kFloat2_GrSLType);
        args.fVaryingHandler->addVarying("atlascoord", &atlasCoord);

        GrGLSLVarying color(kHalf4_GrSLType);
        args.fVaryingHandler->addPassThroughAttribute(
                kInstanceAttribs[2], args.fOutputColor,
                GrGLSLVaryingHandler::Interpolation::kCanBeFlat);

        const char* atlasAdjust;
        fAtlasAdjustUniform = args.fUniformHandler->addUniform(
                kVertex_GrShaderFlag, kFloat2_GrSLType, "atlas_adjust", &atlasAdjust);

        args.fVertBuilder->codeAppendf(R"(
                float2 T = float2(sk_VertexID & 1, sk_VertexID >> 1);
                float2 devcoord = mix(float2(devibounds.xy), float2(devibounds.zw), T);
                float2 atlascoord = devcoord + float2(dev_to_atlas_offset);
                %s = atlascoord * %s;)",
                atlasCoord.vsOut(), atlasAdjust);

        gpArgs->fPositionVar.set(kFloat2_GrSLType, "devcoord");

        GrShaderVar localCoord = gpArgs->fPositionVar;
        if (shader.fUsesLocalCoords) {
            args.fVertBuilder->codeAppendf(R"(
                    float2x2 M = float2x2(viewmatrix_scaleskew);
                    float2 localcoord = inverse(M) * (devcoord - viewmatrix_trans);)");
            localCoord.set(kFloat2_GrSLType, "localcoord");
        }
        this->emitTransforms(args.fVertBuilder, args.fVaryingHandler, args.fUniformHandler,
                             localCoord, args.fFPCoordTransformHandler);

        args.fFragBuilder->codeAppendf("%s = ", args.fOutputCoverage);
        args.fFragBuilder->appendTextureLookup(args.fTexSamplers[0], atlasCoord.fsIn());
        args.fFragBuilder->codeAppendf(".aaaa;");
    }

    void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor& primProc,
                 const CoordTransformRange& transformRange) override {
        const SkISize& dimensions = primProc.cast<DrawAtlasPathShader>().fAtlasDimensions;
        pdman.set2f(fAtlasAdjustUniform, 1.f / dimensions.width(), 1.f / dimensions.height());
        this->setTransformDataHelper(SkMatrix::I(), pdman, transformRange);
    }

    GrGLSLUniformHandler::UniformHandle fAtlasAdjustUniform;
};

GrGLSLPrimitiveProcessor* DrawAtlasPathShader::createGLSLInstance(const GrShaderCaps&) const {
    return new Impl();
}

}  // namespace

GrProcessorSet::Analysis GrDrawAtlasPathOp::finalize(const GrCaps& caps, const GrAppliedClip* clip,
                                                     bool hasMixedSampledCoverage,
                                                     GrClampType clampType) {
    const GrProcessorSet::Analysis& analysis = fProcessors.finalize(
            fInstanceList.fInstance.fColor, GrProcessorAnalysisCoverage::kSingleChannel, clip,
            &GrUserStencilSettings::kUnused, hasMixedSampledCoverage, caps, clampType,
            &fInstanceList.fInstance.fColor);
    fUsesLocalCoords = analysis.usesLocalCoords();
    return analysis;
}

GrOp::CombineResult GrDrawAtlasPathOp::onCombineIfPossible(
        GrOp* op, GrRecordingContext::Arenas* arenas, const GrCaps&) {
    auto* that = op->cast<GrDrawAtlasPathOp>();
    SkASSERT(fAtlasProxy == that->fAtlasProxy);
    SkASSERT(fEnableHWAA == that->fEnableHWAA);

    if (fProcessors != that->fProcessors) {
        return CombineResult::kCannotCombine;
    }

    SkASSERT(fUsesLocalCoords == that->fUsesLocalCoords);
    auto* copy = arenas->recordTimeAllocator()->make<InstanceList>(that->fInstanceList);
    *fInstanceTail = copy;
    fInstanceTail = (!copy->fNext) ? &copy->fNext : that->fInstanceTail;
    fInstanceCount += that->fInstanceCount;
    return CombineResult::kMerged;
}

void GrDrawAtlasPathOp::onPrePrepare(GrRecordingContext*,
                                     const GrSurfaceProxyView* outputView,
                                     GrAppliedClip*,
                                     const GrXferProcessor::DstProxyView&) {
}

void GrDrawAtlasPathOp::onPrepare(GrOpFlushState* state) {
    size_t instanceStride = Instance::Stride(fUsesLocalCoords);
    if (char* instanceData = (char*)state->makeVertexSpace(
            instanceStride, fInstanceCount, &fInstanceBuffer, &fBaseInstance)) {
        SkDEBUGCODE(char* end = instanceData + fInstanceCount * instanceStride);
        for (const InstanceList* list = &fInstanceList; list; list = list->fNext) {
            memcpy(instanceData, &list->fInstance, instanceStride);
            instanceData += instanceStride;
        }
        SkASSERT(instanceData == end);
    }
}

void GrDrawAtlasPathOp::onExecute(GrOpFlushState* state, const SkRect& chainBounds) {
    SkASSERT(fAtlasProxy->isInstantiated());

    GrPipeline::InitArgs initArgs;
    if (fEnableHWAA) {
        initArgs.fInputFlags |= GrPipeline::InputFlags::kHWAntialias;
    }
    initArgs.fCaps = &state->caps();
    initArgs.fDstProxyView = state->drawOpArgs().dstProxyView();
    initArgs.fWriteSwizzle = state->drawOpArgs().writeSwizzle();
    GrPipeline pipeline(initArgs, std::move(fProcessors), state->detachAppliedClip());

    GrSwizzle swizzle = state->caps().getReadSwizzle(fAtlasProxy->backendFormat(),
                                                     GrColorType::kAlpha_8);

    DrawAtlasPathShader shader(fAtlasProxy.get(), swizzle, fUsesLocalCoords);
    SkASSERT(shader.instanceStride() == Instance::Stride(fUsesLocalCoords));

    GrProgramInfo programInfo(state->proxy()->numSamples(), state->proxy()->numStencilSamples(),
                              state->proxy()->backendFormat(), state->outputView()->origin(),
                              &pipeline, &shader, GrPrimitiveType::kTriangleStrip);

    state->bindPipelineAndScissorClip(programInfo, this->bounds());
    state->bindTextures(shader, *fAtlasProxy, pipeline);
    state->bindBuffers(nullptr, fInstanceBuffer.get(), nullptr);
    state->drawInstanced(fInstanceCount, fBaseInstance, 4, 0);
}
