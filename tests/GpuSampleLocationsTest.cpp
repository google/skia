/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"
#include "SkPoint.h"
#include "Test.h"
#include <vector>

#if SK_SUPPORT_GPU

#include "GrAppliedClip.h"
#include "GrRenderTargetContext.h"
#include "GrRenderTargetPriv.h"
#include "GrTypesPriv.h"
#include "gl/GrGLGpu.h"
#include "gl/debug/DebugGLTestContext.h"

typedef std::vector<SkPoint> SamplePattern;

static const SamplePattern kTestPatterns[] = {
    SamplePattern{ // Intel on mac, msaa8, offscreen.
        {0.562500, 0.312500},
        {0.437500, 0.687500},
        {0.812500, 0.562500},
        {0.312500, 0.187500},
        {0.187500, 0.812500},
        {0.062500, 0.437500},
        {0.687500, 0.937500},
        {0.937500, 0.062500}
    },

    SamplePattern{ // Intel on mac, msaa8, on-screen.
        {0.562500, 0.687500},
        {0.437500, 0.312500},
        {0.812500, 0.437500},
        {0.312500, 0.812500},
        {0.187500, 0.187500},
        {0.062500, 0.562500},
        {0.687500, 0.062500},
        {0.937500, 0.937500}
    },

    SamplePattern{ // NVIDIA, msaa16.
        {0.062500, 0.000000},
        {0.250000, 0.125000},
        {0.187500, 0.375000},
        {0.437500, 0.312500},
        {0.500000, 0.062500},
        {0.687500, 0.187500},
        {0.750000, 0.437500},
        {0.937500, 0.250000},
        {0.000000, 0.500000},
        {0.312500, 0.625000},
        {0.125000, 0.750000},
        {0.375000, 0.875000},
        {0.562500, 0.562500},
        {0.812500, 0.687500},
        {0.625000, 0.812500},
        {0.875000, 0.937500}
    },

    SamplePattern{ // NVIDIA, mixed samples, 16:1.
        {0.250000, 0.125000},
        {0.625000, 0.812500},
        {0.500000, 0.062500},
        {0.812500, 0.687500},
        {0.187500, 0.375000},
        {0.875000, 0.937500},
        {0.125000, 0.750000},
        {0.750000, 0.437500},
        {0.937500, 0.250000},
        {0.312500, 0.625000},
        {0.437500, 0.312500},
        {0.000000, 0.500000},
        {0.375000, 0.875000},
        {0.687500, 0.187500},
        {0.062500, 0.000000},
        {0.562500, 0.562500}
    }
};
constexpr int numTestPatterns = SK_ARRAY_COUNT(kTestPatterns);

class TestSampleLocationsInterface : public SkNoncopyable {
public:
    virtual void overrideSamplePattern(const SamplePattern&) = 0;
    virtual ~TestSampleLocationsInterface() {}
};

void assert_equal(skiatest::Reporter* reporter, const SamplePattern& pattern,
                  const GrGpu::MultisampleSpecs& specs, bool flipY) {
    GrAlwaysAssert(specs.fSampleLocations);
    if ((int)pattern.size() != specs.fEffectiveSampleCnt) {
        REPORT_FAILURE(reporter, "", SkString("Sample pattern has wrong number of samples."));
        return;
    }
    for (int i = 0; i < specs.fEffectiveSampleCnt; ++i) {
        SkPoint expectedLocation = specs.fSampleLocations[i];
        if (flipY) {
            expectedLocation.fY = 1 - expectedLocation.fY;
        }
        if (pattern[i] != expectedLocation) {
            REPORT_FAILURE(reporter, "", SkString("Sample pattern has wrong sample location."));
            return;
        }
    }
}

void test_sampleLocations(skiatest::Reporter* reporter, TestSampleLocationsInterface* testInterface,
                          GrContext* ctx) {
    SkRandom rand;
    sk_sp<GrRenderTargetContext> bottomUps[numTestPatterns];
    sk_sp<GrRenderTargetContext> topDowns[numTestPatterns];
    for (int i = 0; i < numTestPatterns; ++i) {
        int numSamples = (int)kTestPatterns[i].size();
        GrAlwaysAssert(numSamples > 1 && SkIsPow2(numSamples));
        bottomUps[i] = ctx->makeDeferredRenderTargetContextWithFallback(
                           SkBackingFit::kExact, 100, 100, kRGBA_8888_GrPixelConfig, nullptr,
                           rand.nextRangeU(1 + numSamples / 2, numSamples),
                           kBottomLeft_GrSurfaceOrigin);
        topDowns[i] = ctx->makeDeferredRenderTargetContextWithFallback(
                          SkBackingFit::kExact, 100, 100, kRGBA_8888_GrPixelConfig, nullptr,
                          rand.nextRangeU(1 + numSamples / 2, numSamples),
                          kTopLeft_GrSurfaceOrigin);
    }

    // Ensure all sample locations get queried and/or cached properly.
    for (int repeat = 0; repeat < 2; ++repeat) {
        for (int i = 0; i < numTestPatterns; ++i) {
            testInterface->overrideSamplePattern(kTestPatterns[i]);
            for (GrRenderTargetContext* rtc : {bottomUps[i].get(), topDowns[i].get()}) {
                GrPipeline dummyPipeline(rtc->accessRenderTarget(),
                                         GrPipeline::ScissorState::kDisabled,
                                         SkBlendMode::kSrcOver);
                GrRenderTarget* rt = rtc->accessRenderTarget();
                assert_equal(reporter, kTestPatterns[i],
                             rt->renderTargetPriv().getMultisampleSpecs(dummyPipeline),
                             kBottomLeft_GrSurfaceOrigin == rt->origin());
            }
        }
    }

}

////////////////////////////////////////////////////////////////////////////////////////////////////

class GLTestSampleLocationsInterface : public TestSampleLocationsInterface, public GrGLInterface {
public:
    GLTestSampleLocationsInterface() : fTestContext(sk_gpu_test::CreateDebugGLTestContext()) {
        fStandard = fTestContext->gl()->fStandard;
        fExtensions = fTestContext->gl()->fExtensions;
        fFunctions = fTestContext->gl()->fFunctions;

        fFunctions.fGetIntegerv = [&](GrGLenum pname, GrGLint* params) {
            GrAlwaysAssert(GR_GL_EFFECTIVE_RASTER_SAMPLES != pname);
            if (GR_GL_SAMPLES == pname) {
                GrAlwaysAssert(!fSamplePattern.empty());
                *params = (int)fSamplePattern.size();
            } else {
                fTestContext->gl()->fFunctions.fGetIntegerv(pname, params);
            }
        };

        fFunctions.fGetMultisamplefv = [&](GrGLenum pname, GrGLuint index, GrGLfloat* val) {
            GrAlwaysAssert(GR_GL_SAMPLE_POSITION == pname);
            val[0] = fSamplePattern[index].fX;
            val[1] = fSamplePattern[index].fY;
        };
    }

    operator GrBackendContext() {
        return reinterpret_cast<GrBackendContext>(static_cast<GrGLInterface*>(this));
    }

    void overrideSamplePattern(const SamplePattern& newPattern) override {
        fSamplePattern = newPattern;
    }

private:
    std::unique_ptr<sk_gpu_test::GLTestContext> fTestContext;
    SamplePattern                               fSamplePattern;
};

DEF_GPUTEST(GLSampleLocations, reporter, /*factory*/) {
    GLTestSampleLocationsInterface testInterface;
    sk_sp<GrContext> ctx(GrContext::Create(kOpenGL_GrBackend, testInterface));

    // This test relies on at least 2 samples.
    int supportedSample = ctx->caps()->getSampleCount(2, kRGBA_8888_GrPixelConfig);
    if (supportedSample < 2) {
        return;
    }
    test_sampleLocations(reporter, &testInterface, ctx.get());
}

#endif
