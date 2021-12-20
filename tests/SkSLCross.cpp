/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/v1/SurfaceDrawContext_v1.h"

static void run_test(skiatest::Reporter*, GrDirectContext*,
                     skgpu::v1::SurfaceDrawContext*, SkVector a,
                     SkVector b, float expectedCrossProduct);

// This is a GPU test that ensures the SkSL 2d cross() intrinsic returns the correct sign (negative,
// positive, or zero).
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SkSLCross, reporter, ctxInfo) {
    GrDirectContext* dContext = ctxInfo.directContext();
    auto sdc = skgpu::v1::SurfaceDrawContext::Make(dContext, GrColorType::kRGBA_8888, nullptr,
                                                   SkBackingFit::kExact, {1, 1}, SkSurfaceProps());
    if (!sdc) {
        ERRORF(reporter, "could not create render target context.");
        return;
    }
    run_test(reporter, dContext, sdc.get(), {3,4}, {5,6}, -2);  // Negative.
    run_test(reporter, dContext, sdc.get(), {3,4}, {-5,-6}, 2);  // Positive.
    run_test(reporter, dContext, sdc.get(), {0, 2.287f}, {0, -7.741f}, 0);  // Zero.
    run_test(reporter, dContext, sdc.get(), {62.17f, 0}, {-43.49f, 0}, 0);  // Zero.
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

    const char* name() const override { return "VisualizeCrossProductSignFP"; }

    std::unique_ptr<GrFragmentProcessor> clone() const override {
        return std::unique_ptr<GrFragmentProcessor>(new VisualizeCrossProductSignFP(fA, fB));
    }

private:
    void onAddToKey(const GrShaderCaps&, skgpu::KeyBuilder*) const override {}
    bool onIsEqual(const GrFragmentProcessor&) const override { return true; }

    std::unique_ptr<ProgramImpl> onMakeProgramImpl() const override {
        class Impl : public ProgramImpl {
        public:
            void emitCode(EmitArgs& args) override {
                auto& fp = args.fFp.cast<VisualizeCrossProductSignFP>();
                const char *a, *b;
                fAUniform = args.fUniformHandler->addUniform(&fp, kFragment_GrShaderFlag,
                                                             GrSLType::kFloat2_GrSLType, "a", &a);
                fBUniform = args.fUniformHandler->addUniform(&fp, kFragment_GrShaderFlag,
                                                             GrSLType::kFloat2_GrSLType, "b", &b);
                args.fFragBuilder->codeAppendf(R"(
                    float crossProduct = cross_length_2d(%s, %s);
                    float2 visualization = clamp(float2(-sign(crossProduct), sign(crossProduct)),
                                                 float2(0), float2(1));
                return half2(visualization).xy01;)", a, b);
            }

        private:
            void onSetData(const GrGLSLProgramDataManager& pdman,
                           const GrFragmentProcessor& processor) override {
                const auto& fp = processor.cast<VisualizeCrossProductSignFP>();
                pdman.set2f(fAUniform, fp.fA.x(), fp.fA.y());
                pdman.set2f(fBUniform, fp.fB.x(), fp.fB.y());
            }
            GrGLSLUniformHandler::UniformHandle fAUniform;
            GrGLSLUniformHandler::UniformHandle fBUniform;
        };

        return std::make_unique<Impl>();
    }
    const SkVector fA, fB;
};

}  // namespace

static void run_test(skiatest::Reporter* reporter, GrDirectContext* directContext,
                     skgpu::v1::SurfaceDrawContext* sdc, SkVector a, SkVector b,
                     float expectedCrossProduct) {
    SkASSERT(sdc->width() == 1);
    SkASSERT(sdc->height() == 1);

    sdc->clear(SkPMColor4f::FromBytes_RGBA(0xbaaaaaad));

    GrPaint crossPaint;
    crossPaint.setColor4f(SK_PMColor4fWHITE);
    crossPaint.setPorterDuffXPFactory(SkBlendMode::kSrcOver);
    crossPaint.setColorFragmentProcessor(std::make_unique<VisualizeCrossProductSignFP>(a, b));
    sdc->drawRect(/*clip=*/nullptr, std::move(crossPaint), GrAA::kNo, SkMatrix::I(),
                  SkRect::MakeWH(1,1));

    GrColor result;
    GrPixmap resultPM(SkImageInfo::Make(1, 1, kRGBA_8888_SkColorType, kPremul_SkAlphaType),
                      &result,
                      sizeof(GrColor));
    sdc->readPixels(directContext, resultPM, {0, 0});

    SkASSERT(expectedCrossProduct == a.cross(b));
    if (expectedCrossProduct > 0) {
        REPORTER_ASSERT(reporter, result == GrColorPackRGBA(0, 255, 0, 255));  // Green.
    } else if (expectedCrossProduct < 0) {
        REPORTER_ASSERT(reporter, result == GrColorPackRGBA(255, 0, 0, 255));  // Red.
    } else {
        REPORTER_ASSERT(reporter, result == GrColorPackRGBA(0, 0, 0, 255));  // Black.
    }
}
