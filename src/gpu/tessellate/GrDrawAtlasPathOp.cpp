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
#include "src/gpu/GrVertexWriter.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"

namespace {

class DrawAtlasPathShader : public GrGeometryProcessor {
public:
    DrawAtlasPathShader(const GrTextureProxy* atlasProxy, GrSwizzle swizzle, bool isInverseFill,
                        bool usesLocalCoords)
            : GrGeometryProcessor(kDrawAtlasPathShader_ClassID)
            , fAtlasAccess(GrSamplerState::Filter::kNearest, atlasProxy->backendFormat(), swizzle)
            , fAtlasDimensions(atlasProxy->backingStoreDimensions())
            , fIsInverseFill(isInverseFill)
            , fUsesLocalCoords(usesLocalCoords) {
        fAttribs.emplace_back("dev_xywh", kInt4_GrVertexAttribType, kInt4_GrSLType);
        fAttribs.emplace_back("atlas_xy", kInt2_GrVertexAttribType, kInt2_GrSLType);
        fAttribs.emplace_back("color", kFloat4_GrVertexAttribType, kHalf4_GrSLType);
        if (fIsInverseFill) {
            fAttribs.emplace_back("drawbounds", kFloat4_GrVertexAttribType, kFloat4_GrSLType);
        }
        if (fUsesLocalCoords) {
            fAttribs.emplace_back("viewmatrix_scaleskew", kFloat4_GrVertexAttribType,
                                  kFloat4_GrSLType);
            fAttribs.emplace_back("viewmatrix_trans", kFloat2_GrVertexAttribType, kFloat2_GrSLType);
        }
        this->setInstanceAttributes(fAttribs.data(), fAttribs.count());
        this->setTextureSamplerCnt(1);
    }

private:
    const char* name() const override { return "DrawAtlasPathShader"; }
    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const override {
        b->add32(fUsesLocalCoords);
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
        args.fVaryingHandler->addVarying("atlascoord", &atlasCoord);

        GrGLSLVarying color(kHalf4_GrSLType);
        args.fFragBuilder->codeAppendf("half4 %s;", args.fOutputColor);
        args.fVaryingHandler->addPassThroughAttribute(
                shader.fAttribs[2], args.fOutputColor,
                GrGLSLVaryingHandler::Interpolation::kCanBeFlat);

        const char* atlasAdjust;
        fAtlasAdjustUniform = args.fUniformHandler->addUniform(
                nullptr, kVertex_GrShaderFlag, kFloat2_GrSLType, "atlas_adjust", &atlasAdjust);

        args.fVertBuilder->codeAppendf(R"(
        float2 T = float2(sk_VertexID & 1, sk_VertexID >> 1);
        float2 devtopleft = float2(dev_xywh.xy);)");

        if (shader.fIsInverseFill) {
            args.fVertBuilder->codeAppendf(R"(
            float2 devcoord = mix(drawbounds.xy, drawbounds.zw, T);)");
        } else {
            args.fVertBuilder->codeAppendf(R"(
            float2 devcoord = abs(float2(dev_xywh.zw)) * T + devtopleft;)");
        }

        args.fVertBuilder->codeAppendf(R"(
        float2 atlascoord = devcoord - devtopleft;
        bool transposed = dev_xywh.w < 0;  // Negative height means the path is transposed.
        if (transposed) {
            atlascoord = atlascoord.yx;
        }
        atlascoord += float2(atlas_xy);
        %s = atlascoord * %s;)",
        atlasCoord.vsOut(), atlasAdjust);

        gpArgs->fPositionVar.set(kFloat2_GrSLType, "devcoord");

        if (shader.fUsesLocalCoords) {
            args.fVertBuilder->codeAppendf(R"(
            float2x2 M = float2x2(viewmatrix_scaleskew);
            float2 localcoord = inverse(M) * (devcoord - viewmatrix_trans);)");
            gpArgs->fLocalCoordVar.set(kFloat2_GrSLType, "localcoord");
        }

        if (shader.fIsInverseFill) {
            GrGLSLVarying atlasBounds(kFloat4_GrSLType);
            args.fVaryingHandler->addVarying("atlasbounds", &atlasBounds,
                                             GrGLSLVaryingHandler::Interpolation::kCanBeFlat);
            args.fVertBuilder->codeAppendf(R"(
            int2 atlas_wh = (transposed) ? abs(dev_xywh.wz) : dev_xywh.zw;
            %s = float4(atlas_xy, atlas_xy + atlas_wh) * %s.xyxy;)", atlasBounds.vsOut(),
                    atlasAdjust);

            args.fFragBuilder->codeAppendf(R"(
            half coverage = 0;
            float2 atlascoord = %s;
            float4 atlasbounds = %s;
            if (all(greaterThan(atlascoord, atlasbounds.xy)) &&
                all(lessThan(atlascoord, atlasbounds.zw))) {
                coverage = )", atlasCoord.fsIn(), atlasBounds.fsIn());
            args.fFragBuilder->appendTextureLookup(args.fTexSamplers[0], "atlascoord");
            args.fFragBuilder->codeAppendf(R"(.a;
            }
            half4 %s = half4(1 - coverage);)", args.fOutputCoverage);
        } else {
            args.fFragBuilder->codeAppendf("half4 %s = ", args.fOutputCoverage);
            args.fFragBuilder->appendTextureLookup(args.fTexSamplers[0], atlasCoord.fsIn());
            args.fFragBuilder->codeAppendf(".aaaa;");
        }
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
            fHeadInstance.fColor, GrProcessorAnalysisCoverage::kSingleChannel, clip,
            &GrUserStencilSettings::kUnused, caps, clampType, &fHeadInstance.fColor);
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
    auto* copy = alloc->make<Instance>(that->fHeadInstance);
    *fTailInstance = copy;
    fTailInstance = (!copy->fNext) ? &copy->fNext : that->fTailInstance;
    fInstanceCount += that->fInstanceCount;
    return CombineResult::kMerged;
}

void GrDrawAtlasPathOp::onPrePrepare(GrRecordingContext*, const GrSurfaceProxyView& writeView,
                                     GrAppliedClip*, const GrDstProxyView&,
                                     GrXferBarrierFlags renderPassXferBarriers,
                                     GrLoadOp colorLoadOp) {
    SK_ABORT("DDL support not implemented for GrDrawAtlasPathOp.");
}

void GrDrawAtlasPathOp::onPrepare(GrOpFlushState* state) {
    SkArenaAlloc* arena = state->allocator();

    GrPipeline::InitArgs initArgs;
    if (fEnableHWAA) {
        initArgs.fInputFlags |= GrPipeline::InputFlags::kHWAntialias;
    }
    initArgs.fCaps = &state->caps();
    initArgs.fDstProxyView = state->drawOpArgs().dstProxyView();
    initArgs.fWriteSwizzle = state->drawOpArgs().writeView().swizzle();
    auto pipeline = arena->make<GrPipeline>(initArgs, std::move(fProcessors),
                                            state->detachAppliedClip());
    GrSwizzle swizzle = state->caps().getReadSwizzle(fAtlasProxy->backendFormat(),
                                                     GrColorType::kAlpha_8);
    auto shader = arena->make<DrawAtlasPathShader>(fAtlasProxy.get(), swizzle, fIsInverseFill,
                                                   fUsesLocalCoords);
    fProgram = arena->make<GrProgramInfo>(state->writeView(), pipeline,
                                          &GrUserStencilSettings::kUnused, shader,
                                          GrPrimitiveType::kTriangleStrip, 0,
                                          state->renderPassBarriers(), state->colorLoadOp());

    if (GrVertexWriter instanceWriter = state->makeVertexSpace(
                shader->instanceStride(), fInstanceCount, &fInstanceBuffer, &fBaseInstance)) {
        for (const Instance* instance = &fHeadInstance; instance; instance = instance->fNext) {
            instanceWriter.write(
                    instance->fDevXYWH,
                    instance->fAtlasXY,
                    instance->fColor,
                    GrVertexWriter::If(fIsInverseFill, instance->fDrawBoundsIfInverseFilled),
                    GrVertexWriter::If(fUsesLocalCoords, instance->fViewMatrixIfUsingLocalCoords));
        }
    }
}

void GrDrawAtlasPathOp::onExecute(GrOpFlushState* state, const SkRect& chainBounds) {
    SkASSERT(fAtlasProxy->isInstantiated());
    state->bindPipelineAndScissorClip(*fProgram, this->bounds());
    state->bindTextures(fProgram->geomProc(), *fAtlasProxy, fProgram->pipeline());
    state->bindBuffers(nullptr, std::move(fInstanceBuffer), nullptr);
    state->drawInstanced(fInstanceCount, fBaseInstance, 4, 0);
}
