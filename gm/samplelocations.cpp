/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/GrTypes.h"
#include "include/private/GrTypesPriv.h"
#include "include/private/SkColorData.h"
#include "src/gpu/GrBuffer.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrColorSpaceXform.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrOpsRenderPass.h"
#include "src/gpu/GrPaint.h"
#include "src/gpu/GrPipeline.h"
#include "src/gpu/GrPrimitiveProcessor.h"
#include "src/gpu/GrProcessor.h"
#include "src/gpu/GrProcessorSet.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrSamplerState.h"
#include "src/gpu/GrShaderCaps.h"
#include "src/gpu/GrShaderVar.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/GrSurfaceProxy.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/GrUserStencilSettings.h"
#include "src/gpu/effects/GrPorterDuffXferProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"
#include "src/gpu/glsl/GrGLSLPrimitiveProcessor.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"
#include "src/gpu/ops/GrDrawOp.h"
#include "src/gpu/ops/GrOp.h"
#include "tools/gpu/ProxyUtils.h"

#include <memory>
#include <utility>

class GrAppliedClip;
class GrGLSLProgramDataManager;

namespace skiagm {

enum class GradType : bool {
    kHW,
    kSW
};

/**
 * This test ensures that the shaderBuilder's sample offsets and sample mask are correlated with
 * actual HW sample locations. It does so by drawing pseudo-random subpixel boxes, and only turning
 * off the samples whose locations fall inside the boxes.
 */
class SampleLocationsGM : public GpuGM {
public:
    SampleLocationsGM(GradType gradType, GrSurfaceOrigin origin)
            : fGradType(gradType)
            , fOrigin(origin) {}

private:
    SkString onShortName() override {
        return SkStringPrintf("samplelocations%s%s",
                              (GradType::kHW == fGradType) ? "_hwgrad" : "_swgrad",
                              (kTopLeft_GrSurfaceOrigin == fOrigin) ? "_topleft" : "_botleft");
    }

    SkISize onISize() override { return SkISize::Make(200, 200); }
    DrawResult onDraw(GrRecordingContext*, GrSurfaceDrawContext*,
                      SkCanvas*, SkString* errorMsg) override;

    const GradType fGradType;
    const GrSurfaceOrigin fOrigin;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// SkSL code.

class SampleLocationsTestProcessor : public GrGeometryProcessor {
public:
    static GrGeometryProcessor* Make(SkArenaAlloc* arena, GradType gradType) {
        return arena->make([&](void* ptr) {
            return new (ptr) SampleLocationsTestProcessor(gradType);
        });
    }

    const char* name() const override { return "SampleLocationsTestProcessor"; }

    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const final {
        b->add32((uint32_t)fGradType);
    }

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const final;

private:
    SampleLocationsTestProcessor(GradType gradType)
            : GrGeometryProcessor(kSampleLocationsTestProcessor_ClassID)
            , fGradType(gradType) {
        this->setWillUseCustomFeature(CustomFeatures::kSampleLocations);
    }

    const GradType fGradType;

    class Impl;

    using INHERITED = GrGeometryProcessor;
};

class SampleLocationsTestProcessor::Impl : public GrGLSLGeometryProcessor {
    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
        const auto& proc = args.fGP.cast<SampleLocationsTestProcessor>();
        auto* v = args.fVertBuilder;
        auto* f = args.fFragBuilder;

        GrGLSLVarying coord(kFloat2_GrSLType);
        GrGLSLVarying grad(kFloat2_GrSLType);
        args.fVaryingHandler->addVarying("coord", &coord);
        if (GradType::kSW == proc.fGradType) {
            args.fVaryingHandler->addVarying("grad", &grad);
        }

        // Pixel grid.
        v->codeAppendf("int x = sk_InstanceID %% 200;");
        v->codeAppendf("int y = sk_InstanceID / 200;");

        // Create pseudo-random rectangles inside a 16x16 subpixel grid. This works out nicely
        // because there are 17 positions on the grid (including both edges), and 17 is a great
        // prime number for generating pseudo-random numbers.
        v->codeAppendf("int ileft = (sk_InstanceID*929) %% 17;");
        v->codeAppendf("int iright = ileft + 1 + ((sk_InstanceID*1637) %% (17 - ileft));");
        v->codeAppendf("int itop = (sk_InstanceID*313) %% 17;");
        v->codeAppendf("int ibot = itop + 1 + ((sk_InstanceID*1901) %% (17 - itop));");

        // Outset (or inset) the rectangle, for the very likely scenario that samples fall on exact
        // 16ths of a pixel. GL_SUBPIXEL_BITS is allowed to be as low as 4, so try not to let the
        // outset value to get too small.
        v->codeAppendf("float outset = 1/32.0;");
        v->codeAppendf("outset = (0 == (x + y) %% 2) ? -outset : +outset;");
        v->codeAppendf("float l = float(ileft)/16.0 - outset;");
        v->codeAppendf("float r = float(iright)/16.0 + outset;");
        v->codeAppendf("float t = float(itop)/16.0 - outset;");
        v->codeAppendf("float b = float(ibot)/16.0 + outset;");

        v->codeAppendf("float2 vertexpos;");
        v->codeAppendf("vertexpos.x = float(x) + ((0 == (sk_VertexID %% 2)) ? l : r);");
        v->codeAppendf("vertexpos.y = float(y) + ((0 == (sk_VertexID / 2)) ? t : b);");
        gpArgs->fPositionVar.set(kFloat2_GrSLType, "vertexpos");

        v->codeAppendf("%s.x = (0 == (sk_VertexID %% 2)) ? -1 : +1;", coord.vsOut());
        v->codeAppendf("%s.y = (0 == (sk_VertexID / 2)) ? -1 : +1;", coord.vsOut());
        if (GradType::kSW == proc.fGradType) {
            v->codeAppendf("%s = 2/float2(r - l, b - t);", grad.vsOut());
        }

        // Fragment shader: Output RED.
        f->codeAppendf("const half4 %s = half4(1,0,0,1);", args.fOutputColor);
        f->codeAppendf("const half4 %s = half4(1);", args.fOutputCoverage);

        // Now turn off all the samples inside our sub-rectangle. As long as the shaderBuilder's
        // sample offsets and sample mask are correlated with actual HW sample locations, no red
        // will bleed through.
        f->codeAppendf("for (int i = 0; i < %i; ++i) {",
                       f->getProgramBuilder()->effectiveSampleCnt());
        if (GradType::kHW == proc.fGradType) {
            f->codeAppendf("float2x2 grad = float2x2(dFdx(%s), dFdy(%s));",
                           coord.fsIn(), coord.fsIn());
        } else {
            f->codeAppendf("float2x2 grad = float2x2(%s.x, 0, 0, %s.y);", grad.fsIn(), grad.fsIn());
        }
        f->codeAppendf(    "float2 samplecoord = %s[i] * grad + %s;",
                           f->sampleOffsets(), coord.fsIn());
        f->codeAppendf(    "if (all(lessThanEqual(abs(samplecoord), float2(1)))) {");
        f->maskOffMultisampleCoverage(
                "~(1 << i)", GrGLSLFPFragmentBuilder::ScopeFlags::kInsideLoop);
        f->codeAppendf(    "}");
        f->codeAppendf("}");
    }

    void setData(const GrGLSLProgramDataManager&, const GrPrimitiveProcessor&) override {}
};

GrGLSLPrimitiveProcessor* SampleLocationsTestProcessor::createGLSLInstance(
        const GrShaderCaps&) const {
    return new Impl();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Draw Op.

static constexpr GrUserStencilSettings gStencilWrite(
    GrUserStencilSettings::StaticInit<
        0x0001,
        GrUserStencilTest::kAlways,
        0xffff,
        GrUserStencilOp::kReplace,
        GrUserStencilOp::kKeep,
        0xffff>()
);

class SampleLocationsTestOp : public GrDrawOp {
public:
    DEFINE_OP_CLASS_ID

    static GrOp::Owner Make(
            GrRecordingContext* ctx, const SkMatrix& viewMatrix, GradType gradType) {
        return GrOp::Make<SampleLocationsTestOp>(ctx, gradType);
    }

private:
    SampleLocationsTestOp(GradType gradType) : GrDrawOp(ClassID()), fGradType(gradType) {
        this->setBounds(SkRect::MakeIWH(200, 200), HasAABloat::kNo, IsHairline::kNo);
    }

    const char* name() const override { return "SampleLocationsTestOp"; }
    FixedFunctionFlags fixedFunctionFlags() const override {
        return FixedFunctionFlags::kUsesHWAA | FixedFunctionFlags::kUsesStencil;
    }
    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*,
                                      bool hasMixedSampledCoverage, GrClampType) override {
        return GrProcessorSet::EmptySetAnalysis();
    }


    GrProgramInfo* createProgramInfo(const GrCaps* caps,
                                     SkArenaAlloc* arena,
                                     const GrSurfaceProxyView& writeView,
                                     GrAppliedClip&& appliedClip,
                                     const GrXferProcessor::DstProxyView& dstProxyView,
                                     GrXferBarrierFlags renderPassXferBarriers,
                                     GrLoadOp colorLoadOp) const {
        GrGeometryProcessor* geomProc = SampleLocationsTestProcessor::Make(arena, fGradType);

        GrPipeline::InputFlags flags = GrPipeline::InputFlags::kHWAntialias;

        return sk_gpu_test::CreateProgramInfo(caps, arena, writeView,
                                              std::move(appliedClip), dstProxyView,
                                              geomProc, SkBlendMode::kSrcOver,
                                              GrPrimitiveType::kTriangleStrip,
                                              renderPassXferBarriers, colorLoadOp,
                                              flags, &gStencilWrite);
    }

    GrProgramInfo* createProgramInfo(GrOpFlushState* flushState) const {
        return this->createProgramInfo(&flushState->caps(),
                                       flushState->allocator(),
                                       flushState->writeView(),
                                       flushState->detachAppliedClip(),
                                       flushState->dstProxyView(),
                                       flushState->renderPassBarriers(),
                                       flushState->colorLoadOp());
    }

    void onPrePrepare(GrRecordingContext* context,
                      const GrSurfaceProxyView& writeView,
                      GrAppliedClip* clip,
                      const GrXferProcessor::DstProxyView& dstProxyView,
                      GrXferBarrierFlags renderPassXferBarriers,
                      GrLoadOp colorLoadOp) final {
        // We're going to create the GrProgramInfo (and the GrPipeline and geometry processor
        // it relies on) in the DDL-record-time arena.
        SkArenaAlloc* arena = context->priv().recordTimeAllocator();

        // This is equivalent to a GrOpFlushState::detachAppliedClip
        GrAppliedClip appliedClip = clip ? std::move(*clip) : GrAppliedClip::Disabled();

        fProgramInfo = this->createProgramInfo(context->priv().caps(), arena, writeView,
                                               std::move(appliedClip), dstProxyView,
                                               renderPassXferBarriers, colorLoadOp);

        context->priv().recordProgramInfo(fProgramInfo);
    }

    void onPrepare(GrOpFlushState*) final {}

    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) final {
        if (!fProgramInfo) {
            fProgramInfo = this->createProgramInfo(flushState);
        }

        flushState->bindPipelineAndScissorClip(*fProgramInfo, SkRect::MakeIWH(200, 200));
        flushState->bindBuffers(nullptr, nullptr, nullptr);
        flushState->drawInstanced(200*200, 0, 4, 0);
    }

    const GradType fGradType;

    // The program info (and both the GrPipeline and GrPrimitiveProcessor it relies on), when
    // allocated, are allocated in either the ddl-record-time or flush-time arena. It is the
    // arena's job to free up their memory so we just have a bare programInfo pointer here. We
    // don't even store the GrPipeline and GrPrimitiveProcessor pointers here bc they are
    // guaranteed to have the same lifetime as the program info.
    GrProgramInfo*  fProgramInfo = nullptr;

    friend class ::GrOp; // for ctor

    using INHERITED = GrDrawOp;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Test.

DrawResult SampleLocationsGM::onDraw(GrRecordingContext* ctx, GrSurfaceDrawContext* rtc,
                                     SkCanvas* canvas, SkString* errorMsg) {
    if (!ctx->priv().caps()->sampleLocationsSupport()) {
        *errorMsg = "Requires support for sample locations.";
        return DrawResult::kSkip;
    }
    if (!ctx->priv().caps()->shaderCaps()->sampleMaskSupport()) {
        *errorMsg = "Requires support for sample mask.";
        return DrawResult::kSkip;
    }
    if (!ctx->priv().caps()->drawInstancedSupport()) {
        *errorMsg = "Requires support for instanced rendering.";
        return DrawResult::kSkip;
    }
    if (rtc->numSamples() <= 1 && !ctx->priv().caps()->mixedSamplesSupport()) {
        *errorMsg = "MSAA and mixed samples only.";
        return DrawResult::kSkip;
    }

    auto offscreenRTC = GrSurfaceDrawContext::Make(
            ctx, rtc->colorInfo().colorType(), nullptr, SkBackingFit::kExact, {200, 200},
            rtc->numSamples(), GrMipmapped::kNo, GrProtected::kNo, fOrigin);
    if (!offscreenRTC) {
        *errorMsg = "Failed to create offscreen render target.";
        return DrawResult::kFail;
    }
    if (offscreenRTC->numSamples() <= 1 &&
        !offscreenRTC->asRenderTargetProxy()->canUseMixedSamples(*ctx->priv().caps())) {
        *errorMsg = "MSAA and mixed samples only.";
        return DrawResult::kSkip;
    }

    static constexpr GrUserStencilSettings kStencilCover(
        GrUserStencilSettings::StaticInit<
            0x0000,
            GrUserStencilTest::kNotEqual,
            0xffff,
            GrUserStencilOp::kZero,
            GrUserStencilOp::kKeep,
            0xffff>()
    );

    offscreenRTC->clear(SkPMColor4f{0, 1, 0, 1});

    // Stencil.
    offscreenRTC->addDrawOp(SampleLocationsTestOp::Make(ctx, canvas->getTotalMatrix(), fGradType));

    // Cover.
    GrPaint coverPaint;
    coverPaint.setColor4f({1,0,0,1});
    coverPaint.setXPFactory(GrPorterDuffXPFactory::Get(SkBlendMode::kSrcOver));
    rtc->stencilRect(nullptr, &kStencilCover, std::move(coverPaint), GrAA::kNo, SkMatrix::I(),
                     SkRect::MakeWH(200, 200));

    // Copy offscreen texture to canvas.
    rtc->drawTexture(nullptr,
                     offscreenRTC->readSurfaceView(),
                     offscreenRTC->colorInfo().alphaType(),
                     GrSamplerState::Filter::kNearest,
                     GrSamplerState::MipmapMode::kNone,
                     SkBlendMode::kSrc,
                     SK_PMColor4fWHITE,
                     {0, 0, 200, 200},
                     {0, 0, 200, 200},
                     GrAA::kNo,
                     GrQuadAAFlags::kNone,
                     SkCanvas::SrcRectConstraint::kStrict_SrcRectConstraint,
                     SkMatrix::I(),
                     nullptr);

    return skiagm::DrawResult::kOk;
}

DEF_GM( return new SampleLocationsGM(GradType::kHW, kTopLeft_GrSurfaceOrigin); )
DEF_GM( return new SampleLocationsGM(GradType::kHW, kBottomLeft_GrSurfaceOrigin); )
DEF_GM( return new SampleLocationsGM(GradType::kSW, kTopLeft_GrSurfaceOrigin); )
DEF_GM( return new SampleLocationsGM(GradType::kSW, kBottomLeft_GrSurfaceOrigin); )

}  // namespace skiagm
