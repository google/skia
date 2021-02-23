/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"

static void run_test(skiatest::Reporter*, GrDirectContext*,
                     GrSurfaceDrawContext*, SkVector a,
                     SkVector b, float expectedCrossProduct);

// This is a GPU test that ensures the SkSL 2d cross() intrinsic returns the correct sign (negative,
// positive, or zero).
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SkSLCross, reporter, ctxInfo) {
    GrDirectContext* directContext = ctxInfo.directContext();
    auto rtc = GrSurfaceDrawContext::Make(directContext, GrColorType::kRGBA_8888, nullptr,
                                          SkBackingFit::kExact, {1, 1});
    if (!rtc) {
        ERRORF(reporter, "could not create render target context.");
        return;
    }
    run_test(reporter, directContext, rtc.get(), {3,4}, {5,6}, -2);  // Negative.
    run_test(reporter, directContext, rtc.get(), {3,4}, {-5,-6}, 2);  // Positive.
    run_test(reporter, directContext, rtc.get(), {0, 2.287f}, {0, -7.741f}, 0);  // Zero.
    run_test(reporter, directContext, rtc.get(), {62.17f, 0}, {-43.49f, 0}, 0);  // Zero.
}

namespace {

// Outputs:
//     Green if cross(a,b) > 0
//     Red if cross(a,b) < 0
//     Black if cross(a,b) == 0
class VisualizeCrossProductSignFP : public GrFragmentProcessor {
public:
    VisualizeCrossProductSignFP(SkVector a, SkVector b)
            : GrFragmentProcessor(kTestFP_ClassID, kPreservesOpaqueInput_OptimizationFlag)
            , fA(a), fB(b) {
    }

private:
    const char* name() const override { return "VisualizeCrossProductSignFP"; }
    std::unique_ptr<GrFragmentProcessor> clone() const override {
        return std::unique_ptr<GrFragmentProcessor>(new VisualizeCrossProductSignFP(fA, fB));
    }
    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {}
    bool onIsEqual(const GrFragmentProcessor&) const override { return true; }

    class Impl : public GrGLSLFragmentProcessor {
        void emitCode(EmitArgs& args) override {
            auto& fp = args.fFp.cast<VisualizeCrossProductSignFP>();
            const char* a, *b;
            fAUniform = args.fUniformHandler->addUniform(&fp, kFragment_GrShaderFlag,
                                                         GrSLType::kFloat2_GrSLType, "a", &a);
            fBUniform = args.fUniformHandler->addUniform(&fp, kFragment_GrShaderFlag,
                                                         GrSLType::kFloat2_GrSLType, "b", &b);
            args.fFragBuilder->codeAppendf(R"(
                    float crossProduct = cross(%s, %s);
                    float2 visualization = clamp(float2(-sign(crossProduct), sign(crossProduct)),
                                                 float2(0), float2(1));
                    return half2(visualization).xy01;)", a, b);
        }
        void onSetData(const GrGLSLProgramDataManager& pdman,
                       const GrFragmentProcessor& processor) override {
            const auto& fp = processor.cast<VisualizeCrossProductSignFP>();
            pdman.set2f(fAUniform, fp.fA.x(), fp.fA.y());
            pdman.set2f(fBUniform, fp.fB.x(), fp.fB.y());
        }
        GrGLSLUniformHandler::UniformHandle fAUniform;
        GrGLSLUniformHandler::UniformHandle fBUniform;
    };

    std::unique_ptr<GrGLSLFragmentProcessor> onMakeProgramImpl() const override {
        return std::make_unique<Impl>();
    }
    const SkVector fA, fB;
};

}  // namespace

static void run_test(skiatest::Reporter* reporter, GrDirectContext* directContext,
                     GrSurfaceDrawContext* rtc, SkVector a, SkVector b,
                     float expectedCrossProduct) {
    SkASSERT(rtc->width() == 1);
    SkASSERT(rtc->height() == 1);

    rtc->clear(SkPMColor4f::FromBytes_RGBA(0xbaaaaaad));

    GrPaint crossPaint;
    crossPaint.setColor4f(SK_PMColor4fWHITE);
    crossPaint.setPorterDuffXPFactory(SkBlendMode::kSrcOver);
    crossPaint.setColorFragmentProcessor(std::make_unique<VisualizeCrossProductSignFP>(a, b));
    rtc->drawRect(/*clip=*/nullptr, std::move(crossPaint), GrAA::kNo, SkMatrix::I(),
                  SkRect::MakeWH(1,1));

    GrColor result;
    GrPixmap resultPM(SkImageInfo::Make(1, 1, kRGBA_8888_SkColorType, kPremul_SkAlphaType),
                      &result,
                      sizeof(GrColor));
    rtc->readPixels(directContext, resultPM, {0, 0});

    SkASSERT(expectedCrossProduct == a.cross(b));
    if (expectedCrossProduct > 0) {
        REPORTER_ASSERT(reporter, result == GrColorPackRGBA(0, 255, 0, 255));  // Green.
    } else if (expectedCrossProduct < 0) {
        REPORTER_ASSERT(reporter, result == GrColorPackRGBA(255, 0, 0, 255));  // Red.
    } else {
        REPORTER_ASSERT(reporter, result == GrColorPackRGBA(0, 0, 0, 255));  // Black.
    }
}
