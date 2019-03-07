/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#if SK_SUPPORT_GPU

#include "GrClip.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrMemoryPool.h"
#include "GrOpFlushState.h"
#include "GrRecordingContext.h"
#include "GrRecordingContextPriv.h"
#include "GrRenderTargetContext.h"
#include "GrRenderTargetContextPriv.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLProgramBuilder.h"
#include "glsl/GrGLSLVarying.h"
#include "glsl/GrGLSLVertexGeoBuilder.h"

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
    SkString onShortName() override;
    SkISize onISize() override { return SkISize::Make(200, 200); }
    DrawResult onDraw(GrContext*, GrRenderTargetContext*, SkCanvas*, SkString* errorMsg) override;

    const GradType fGradType;
    const GrSurfaceOrigin fOrigin;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// SkSL code.

class SampleLocationsTestProcessor : public GrGeometryProcessor {
public:
    SampleLocationsTestProcessor(GradType gradType)
            : GrGeometryProcessor(kSampleLocationsTestProcessor_ClassID)
            , fGradType(gradType) {
        this->setWillUseCustomFeature(CustomFeatures::kSampleLocations);
    }
    const char* name() const override { return "SampleLocationsTestProcessor"; }
    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const final {
        b->add32((uint32_t)fGradType);
    }
    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const final;

private:
    const GradType fGradType;

    class Impl;
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
        v->codeAppendf("float l = ileft/16.0 - outset;");
        v->codeAppendf("float r = iright/16.0 + outset;");
        v->codeAppendf("float t = itop/16.0 - outset;");
        v->codeAppendf("float b = ibot/16.0 + outset;");

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
        f->codeAppendf("%s = half4(1,0,0,1);", args.fOutputColor);
        f->codeAppendf("%s = half4(1);", args.fOutputCoverage);

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
                "~(1 << i)", GrGLSLFragmentShaderBuilder::Scope::kInsideLoopOrBranch);
        f->codeAppendf(    "}");
        f->codeAppendf("}");
    }

    void setData(const GrGLSLProgramDataManager&, const GrPrimitiveProcessor&,
                 FPCoordTransformIter&&) override {}
};

GrGLSLPrimitiveProcessor* SampleLocationsTestProcessor::createGLSLInstance(
        const GrShaderCaps&) const {
    return new Impl();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Draw Op.

class SampleLocationsTestOp : public GrDrawOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrDrawOp> Make(
            GrRecordingContext* ctx, const SkMatrix& viewMatrix, GradType gradType) {
        GrOpMemoryPool* pool = ctx->priv().opMemoryPool();
        return pool->allocate<SampleLocationsTestOp>(gradType);
    }

private:
    SampleLocationsTestOp(GradType gradType) : GrDrawOp(ClassID()), fGradType(gradType) {
        this->setBounds(SkRect::MakeIWH(200, 200), HasAABloat::kNo, IsZeroArea::kNo);
    }

    const char* name() const override { return "SampleLocationsTestOp"; }
    FixedFunctionFlags fixedFunctionFlags() const override {
        return FixedFunctionFlags::kUsesHWAA | FixedFunctionFlags::kUsesStencil;
    }
    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*, GrFSAAType) override {
        return GrProcessorSet::EmptySetAnalysis();
    }
    void onPrepare(GrOpFlushState*) override {}
    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override {
        static constexpr GrUserStencilSettings kStencilWrite(
            GrUserStencilSettings::StaticInit<
                0x0001,
                GrUserStencilTest::kAlways,
                0xffff,
                GrUserStencilOp::kReplace,
                GrUserStencilOp::kKeep,
                0xffff>()
        );

        GrPipeline pipeline(GrScissorTest::kDisabled, SkBlendMode::kSrcOver,
                            GrPipeline::Flags::kHWAntialias_Flag, &kStencilWrite);
        GrMesh mesh(GrPrimitiveType::kTriangleStrip);
        mesh.setInstanced(nullptr, 200*200, 0, 4);
        flushState->rtCommandBuffer()->draw(
                SampleLocationsTestProcessor(fGradType), pipeline, nullptr, nullptr, &mesh, 1,
                SkRect::MakeIWH(200, 200));
    }

    const GradType fGradType;

    friend class ::GrOpMemoryPool; // for ctor
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Test.

SkString SampleLocationsGM::onShortName() {
    SkString name("samplelocations");
    name.append((GradType::kHW == fGradType) ? "_hwgrad" : "_swgrad");
    name.append((kTopLeft_GrSurfaceOrigin == fOrigin) ? "_topleft" : "_botleft");
    return name;
}

DrawResult SampleLocationsGM::onDraw(
        GrContext* ctx, GrRenderTargetContext* rtc, SkCanvas* canvas, SkString* errorMsg) {
    if (rtc->numStencilSamples() <= 1) {
        *errorMsg = "MSAA only.";
        return DrawResult::kSkip;
    }
    if (!ctx->priv().caps()->sampleLocationsSupport()) {
        *errorMsg = "Requires support for sample locations.";
        return DrawResult::kSkip;
    }
    if (!ctx->priv().caps()->shaderCaps()->sampleVariablesSupport()) {
        *errorMsg = "Requires support for sample variables.";
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

    if (auto offscreenRTC = ctx->priv().makeDeferredRenderTargetContext(
            rtc->asSurfaceProxy()->backendFormat(), SkBackingFit::kExact, 200, 200,
            rtc->asSurfaceProxy()->config(), nullptr, rtc->numStencilSamples(), GrMipMapped::kNo,
            fOrigin)) {
        offscreenRTC->clear(nullptr, {0,1,0,1}, GrRenderTargetContext::CanClearFullscreen::kYes);

        // Stencil.
        offscreenRTC->priv().testingOnly_addDrawOp(
                SampleLocationsTestOp::Make(ctx, canvas->getTotalMatrix(), fGradType));

        // Cover.
        GrPaint coverPaint;
        coverPaint.setColor4f({1,0,0,1});
        coverPaint.setXPFactory(GrPorterDuffXPFactory::Get(SkBlendMode::kSrcOver));
        rtc->priv().drawFilledRect(
                GrNoClip(), std::move(coverPaint), GrAA::kNo, SkMatrix::I(),
                SkRect::MakeWH(200, 200), &kStencilCover);

        // Copy offscreen texture to canvas.
        rtc->drawTexture(
                GrNoClip(), sk_ref_sp(offscreenRTC->asTextureProxy()),
                GrSamplerState::Filter::kNearest, SkBlendMode::kSrc, SK_PMColor4fWHITE,
                {0,0,200,200}, {0,0,200,200}, GrAA::kNo, GrQuadAAFlags::kNone,
                SkCanvas::SrcRectConstraint::kStrict_SrcRectConstraint, SkMatrix::I(), nullptr);
    }

    return skiagm::DrawResult::kOk;
}

DEF_GM( return new SampleLocationsGM(GradType::kHW, kTopLeft_GrSurfaceOrigin); )
DEF_GM( return new SampleLocationsGM(GradType::kHW, kBottomLeft_GrSurfaceOrigin); )
DEF_GM( return new SampleLocationsGM(GradType::kSW, kTopLeft_GrSurfaceOrigin); )
DEF_GM( return new SampleLocationsGM(GradType::kSW, kBottomLeft_GrSurfaceOrigin); )

}

#endif  // SK_SUPPORT_GPU
