/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "tests/Test.h"

#include "include/gpu/GrContext.h"
#include "include/gpu/GrGpuResource.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrRenderTargetContextPriv.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/ops/GrFillRectOp.h"
#include "src/gpu/ops/GrMeshDrawOp.h"
#include "tests/TestUtils.h"

#include <atomic>
#include <random>

namespace {
class TestOp : public GrMeshDrawOp {
public:
    DEFINE_OP_CLASS_ID
    static std::unique_ptr<GrDrawOp> Make(GrContext* context,
                                          std::unique_ptr<GrFragmentProcessor> fp) {
        GrOpMemoryPool* pool = context->priv().opMemoryPool();

        return pool->allocate<TestOp>(std::move(fp));
    }

    const char* name() const override { return "TestOp"; }

    void visitProxies(const VisitProxyFunc& func) const override {
        fProcessors.visitProxies(func);
    }

    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }

    GrProcessorSet::Analysis finalize(
            const GrCaps& caps, const GrAppliedClip* clip, bool hasMixedSampledCoverage,
            GrClampType clampType) override {
        static constexpr GrProcessorAnalysisColor kUnknownColor;
        SkPMColor4f overrideColor;
        return fProcessors.finalize(
                kUnknownColor, GrProcessorAnalysisCoverage::kNone, clip,
                &GrUserStencilSettings::kUnused, hasMixedSampledCoverage, caps, clampType,
                &overrideColor);
    }

private:
    friend class ::GrOpMemoryPool; // for ctor

    TestOp(std::unique_ptr<GrFragmentProcessor> fp)
            : INHERITED(ClassID()), fProcessors(std::move(fp)) {
        this->setBounds(SkRect::MakeWH(100, 100), HasAABloat::kNo, IsZeroArea::kNo);
    }

    void onPrepareDraws(Target* target) override { return; }
    void onExecute(GrOpFlushState*, const SkRect&) override { return; }

    GrProcessorSet fProcessors;

    typedef GrMeshDrawOp INHERITED;
};

/**
 * FP used to test ref counts on owned GrGpuResources. Can also be a parent FP to test counts
 * of resources owned by child FPs.
 */
class TestFP : public GrFragmentProcessor {
public:
    static std::unique_ptr<GrFragmentProcessor> Make(std::unique_ptr<GrFragmentProcessor> child) {
        return std::unique_ptr<GrFragmentProcessor>(new TestFP(std::move(child)));
    }
    static std::unique_ptr<GrFragmentProcessor> Make(const SkTArray<sk_sp<GrTextureProxy>>& proxies,
                                                     const SkTArray<sk_sp<GrGpuBuffer>>& buffers) {
        return std::unique_ptr<GrFragmentProcessor>(new TestFP(proxies, buffers));
    }

    const char* name() const override { return "test"; }

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const override {
        static std::atomic<int32_t> nextKey{0};
        b->add32(nextKey++);
    }

    std::unique_ptr<GrFragmentProcessor> clone() const override {
        return std::unique_ptr<GrFragmentProcessor>(new TestFP(*this));
    }

private:
    TestFP(const SkTArray<sk_sp<GrTextureProxy>>& proxies,
           const SkTArray<sk_sp<GrGpuBuffer>>& buffers)
            : INHERITED(kTestFP_ClassID, kNone_OptimizationFlags), fSamplers(4) {
        for (const auto& proxy : proxies) {
            fSamplers.emplace_back(proxy);
        }
        this->setTextureSamplerCnt(fSamplers.count());
    }

    TestFP(std::unique_ptr<GrFragmentProcessor> child)
            : INHERITED(kTestFP_ClassID, kNone_OptimizationFlags), fSamplers(4) {
        this->registerChildProcessor(std::move(child));
    }

    explicit TestFP(const TestFP& that)
            : INHERITED(kTestFP_ClassID, that.optimizationFlags()), fSamplers(4) {
        for (int i = 0; i < that.fSamplers.count(); ++i) {
            fSamplers.emplace_back(that.fSamplers[i]);
        }
        for (int i = 0; i < that.numChildProcessors(); ++i) {
            this->registerChildProcessor(that.childProcessor(i).clone());
        }
        this->setTextureSamplerCnt(fSamplers.count());
    }

    virtual GrGLSLFragmentProcessor* onCreateGLSLInstance() const override {
        class TestGLSLFP : public GrGLSLFragmentProcessor {
        public:
            TestGLSLFP() {}
            void emitCode(EmitArgs& args) override {
                GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
                fragBuilder->codeAppendf("%s = %s;", args.fOutputColor, args.fInputColor);
            }

        private:
        };
        return new TestGLSLFP();
    }

    bool onIsEqual(const GrFragmentProcessor&) const override { return false; }
    const TextureSampler& onTextureSampler(int i) const override { return fSamplers[i]; }

    GrTAllocator<TextureSampler> fSamplers;
    typedef GrFragmentProcessor INHERITED;
};
}

static void check_refs(skiatest::Reporter* reporter,
                       GrTextureProxy* proxy,
                       int32_t expectedProxyRefs,
                       int32_t expectedBackingRefs) {
    int32_t actualProxyRefs = proxy->priv().getProxyRefCnt();
    int32_t actualBackingRefs = proxy->testingOnly_getBackingRefCnt();

    SkASSERT(actualProxyRefs == expectedProxyRefs);
    SkASSERT(actualBackingRefs == expectedBackingRefs);

    REPORTER_ASSERT(reporter, actualProxyRefs == expectedProxyRefs);
    REPORTER_ASSERT(reporter, actualBackingRefs == expectedBackingRefs);
}

DEF_GPUTEST_FOR_ALL_CONTEXTS(ProcessorRefTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    GrProxyProvider* proxyProvider = context->priv().proxyProvider();

    GrSurfaceDesc desc;
    desc.fWidth = 10;
    desc.fHeight = 10;
    desc.fConfig = kRGBA_8888_GrPixelConfig;

    const GrBackendFormat format =
            context->priv().caps()->getBackendFormatFromColorType(kRGBA_8888_SkColorType);

    for (bool makeClone : {false, true}) {
        for (int parentCnt = 0; parentCnt < 2; parentCnt++) {
            sk_sp<GrRenderTargetContext> renderTargetContext(
                    context->priv().makeDeferredRenderTargetContext(
                            format, SkBackingFit::kApprox, 1, 1, kRGBA_8888_GrPixelConfig,
                            GrColorType::kRGBA_8888, nullptr));
            {
                sk_sp<GrTextureProxy> proxy = proxyProvider->createProxy(
                        format, desc, kTopLeft_GrSurfaceOrigin, SkBackingFit::kExact,
                        SkBudgeted::kYes);

                {
                    SkTArray<sk_sp<GrTextureProxy>> proxies;
                    SkTArray<sk_sp<GrGpuBuffer>> buffers;
                    proxies.push_back(proxy);
                    auto fp = TestFP::Make(std::move(proxies), std::move(buffers));
                    for (int i = 0; i < parentCnt; ++i) {
                        fp = TestFP::Make(std::move(fp));
                    }
                    std::unique_ptr<GrFragmentProcessor> clone;
                    if (makeClone) {
                        clone = fp->clone();
                    }
                    std::unique_ptr<GrDrawOp> op(TestOp::Make(context, std::move(fp)));
                    renderTargetContext->priv().testingOnly_addDrawOp(std::move(op));
                    if (clone) {
                        op = TestOp::Make(context, std::move(clone));
                        renderTargetContext->priv().testingOnly_addDrawOp(std::move(op));
                    }
                }

                // If the fp is cloned the number of refs should increase by one (for the clone)
                int expectedProxyRefs = makeClone ? 3 : 2;

                check_refs(reporter, proxy.get(), expectedProxyRefs, -1);

                context->flush();

                check_refs(reporter, proxy.get(), 1, 1); // just one from the 'proxy' sk_sp
            }
        }
    }
}

#include "tools/flags/CommandLineFlags.h"
static DEFINE_bool(randomProcessorTest, false,
                   "Use non-deterministic seed for random processor tests?");
static DEFINE_int(processorSeed, 0,
                  "Use specific seed for processor tests. Overridden by --randomProcessorTest.");

#if GR_TEST_UTILS

static GrColor input_texel_color(int i, int j, SkScalar delta) {
    // Delta must be less than 0.5 to prevent over/underflow issues with the input color
    SkASSERT(delta <= 0.5);

    SkColor color = SkColorSetARGB((uint8_t)(i & 0xFF),
                                   (uint8_t)(j & 0xFF),
                                   (uint8_t)((i + j) & 0xFF),
                                   (uint8_t)((2 * j - i) & 0xFF));
    SkColor4f color4f = SkColor4f::FromColor(color);
    for (int i = 0; i < 4; i++) {
        if (color4f[i] > 0.5) {
            color4f[i] -= delta;
        } else {
            color4f[i] += delta;
        }
    }
    return color4f.premul().toBytes_RGBA();
}

void test_draw_op(GrContext* context,
                  GrRenderTargetContext* rtc,
                  std::unique_ptr<GrFragmentProcessor> fp,
                  sk_sp<GrTextureProxy> inputDataProxy) {
    GrPaint paint;
    paint.addColorTextureProcessor(std::move(inputDataProxy), SkMatrix::I());
    paint.addColorFragmentProcessor(std::move(fp));
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);

    auto op = GrFillRectOp::MakeNonAARect(context, std::move(paint), SkMatrix::I(),
                                          SkRect::MakeWH(rtc->width(), rtc->height()));
    rtc->addDrawOp(GrNoClip(), std::move(op));
}

// This assumes that the output buffer will be the same size as inputDataProxy
void render_fp(GrContext* context, GrRenderTargetContext* rtc, GrFragmentProcessor* fp,
               sk_sp<GrTextureProxy> inputDataProxy, GrColor* buffer) {
    int width = inputDataProxy->width();
    int height = inputDataProxy->height();

    // test_draw_op needs to take ownership of an FP, so give it a clone that it can own
    test_draw_op(context, rtc, fp->clone(), inputDataProxy);
    memset(buffer, 0x0, sizeof(GrColor) * width * height);
           rtc->readPixels(SkImageInfo::Make(width, height, kRGBA_8888_SkColorType,
                                             kPremul_SkAlphaType),
                           buffer, 0, 0, 0);
}

/** Initializes the two test texture proxies that are available to the FP test factories. */
bool init_test_textures(GrResourceProvider* resourceProvider,
                        GrProxyProvider* proxyProvider,
                        SkRandom* random,
                        sk_sp<GrTextureProxy> proxies[2]) {
    static const int kTestTextureSize = 256;

    {
        // Put premul data into the RGBA texture that the test FPs can optionally use.
        std::unique_ptr<GrColor[]> rgbaData(new GrColor[kTestTextureSize * kTestTextureSize]);
        for (int y = 0; y < kTestTextureSize; ++y) {
            for (int x = 0; x < kTestTextureSize; ++x) {
                rgbaData[kTestTextureSize * y + x] = input_texel_color(
                        random->nextULessThan(256), random->nextULessThan(256), 0.0f);
            }
        }

        SkImageInfo ii = SkImageInfo::Make(kTestTextureSize, kTestTextureSize,
                                           kRGBA_8888_SkColorType, kPremul_SkAlphaType);
        SkPixmap pixmap(ii, rgbaData.get(), ii.minRowBytes());
        sk_sp<SkImage> img = SkImage::MakeRasterCopy(pixmap);
        proxies[0] = proxyProvider->createTextureProxy(img, kNone_GrSurfaceFlags, 1,
                                                       SkBudgeted::kYes, SkBackingFit::kExact);
        proxies[0]->instantiate(resourceProvider);
    }

    {
        // Put random values into the alpha texture that the test FPs can optionally use.
        std::unique_ptr<uint8_t[]> alphaData(new uint8_t[kTestTextureSize * kTestTextureSize]);
        for (int y = 0; y < kTestTextureSize; ++y) {
            for (int x = 0; x < kTestTextureSize; ++x) {
                alphaData[kTestTextureSize * y + x] = random->nextULessThan(256);
            }
        }

        SkImageInfo ii = SkImageInfo::Make(kTestTextureSize, kTestTextureSize,
                                           kAlpha_8_SkColorType, kPremul_SkAlphaType);
        SkPixmap pixmap(ii, alphaData.get(), ii.minRowBytes());
        sk_sp<SkImage> img = SkImage::MakeRasterCopy(pixmap);
        proxies[1] = proxyProvider->createTextureProxy(img, kNone_GrSurfaceFlags, 1,
                                                       SkBudgeted::kYes, SkBackingFit::kExact);
        proxies[1]->instantiate(resourceProvider);
    }

    return proxies[0] && proxies[1];
}

// Creates a texture of premul colors used as the output of the fragment processor that precedes
// the fragment processor under test. Color values are those provided by input_texel_color().
sk_sp<GrTextureProxy> make_input_texture(GrProxyProvider* proxyProvider, int width, int height,
                                         SkScalar delta) {
    std::unique_ptr<GrColor[]> data(new GrColor[width * height]);
    for (int y = 0; y < width; ++y) {
        for (int x = 0; x < height; ++x) {
            data.get()[width * y + x] = input_texel_color(x, y, delta);
        }
    }

    SkImageInfo ii = SkImageInfo::Make(width, height, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    SkPixmap pixmap(ii, data.get(), ii.minRowBytes());
    sk_sp<SkImage> img = SkImage::MakeRasterCopy(pixmap);
    return proxyProvider->createTextureProxy(img, kNone_GrSurfaceFlags, 1,
                                             SkBudgeted::kYes, SkBackingFit::kExact);
}

bool log_surface_context(sk_sp<GrSurfaceContext> src, SkString* dst) {
    SkImageInfo ii = SkImageInfo::Make(src->width(), src->height(), kRGBA_8888_SkColorType,
                                       kPremul_SkAlphaType);
    SkBitmap bm;
    SkAssertResult(bm.tryAllocPixels(ii));
    SkAssertResult(src->readPixels(ii, bm.getPixels(), bm.rowBytes(), 0, 0));

    return bitmap_to_base64_data_uri(bm, dst);
}

bool log_surface_proxy(GrContext* context, sk_sp<GrSurfaceProxy> src, SkString* dst) {
    // All the inputs are made from kRGBA_8888_SkColorType/kPremul_SkAlphaType bitmaps.
    sk_sp<GrSurfaceContext> sContext(context->priv().makeWrappedSurfaceContext(
            src, GrColorType::kRGBA_8888, kPremul_SkAlphaType));
    return log_surface_context(sContext, dst);
}

bool fuzzy_color_equals(const SkPMColor4f& c1, const SkPMColor4f& c2) {
    // With the loss of precision of rendering into 32-bit color, then estimating the FP's output
    // from that, it is not uncommon for a valid output to differ from estimate by up to 0.01
    // (really 1/128 ~ .0078, but frequently floating point issues make that tolerance a little
    // too unforgiving).
    static constexpr SkScalar kTolerance = 0.01f;
    for (int i = 0; i < 4; i++) {
        if (!SkScalarNearlyEqual(c1[i], c2[i], kTolerance)) {
            return false;
        }
    }
    return true;
}

int modulation_index(int channelIndex, bool alphaModulation) {
    return alphaModulation ? 3 : channelIndex;
}

// Given three input colors (color preceding the FP being tested), and the output of the FP, this
// ensures that the out1 = fp * in1.a, out2 = fp * in2.a, and out3 = fp * in3.a, where fp is the
// pre-modulated color that should not be changing across frames (FP's state doesn't change).
//
// When alphaModulation is false, this tests the very similar conditions that out1 = fp * in1,
// etc. using per-channel modulation instead of modulation by just the input alpha channel.
// - This estimates the pre-modulated fp color from one of the input/output pairs and confirms the
//   conditions hold for the other two pairs.
bool legal_modulation(const GrColor& in1, const GrColor& in2, const GrColor& in3,
                      const GrColor& out1, const GrColor& out2, const GrColor& out3,
                      bool alphaModulation) {
    // Convert to floating point, which is the number space the FP operates in (more or less)
    SkPMColor4f in1f = SkPMColor4f::FromBytes_RGBA(in1);
    SkPMColor4f in2f = SkPMColor4f::FromBytes_RGBA(in2);
    SkPMColor4f in3f = SkPMColor4f::FromBytes_RGBA(in3);
    SkPMColor4f out1f = SkPMColor4f::FromBytes_RGBA(out1);
    SkPMColor4f out2f = SkPMColor4f::FromBytes_RGBA(out2);
    SkPMColor4f out3f = SkPMColor4f::FromBytes_RGBA(out3);

    // Reconstruct the output of the FP before the shader modulated its color with the input value.
    // When the original input is very small, it may cause the final output color to round
    // to 0, in which case we estimate the pre-modulated color using one of the stepped frames that
    // will then have a guaranteed larger channel value (since the offset will be added to it).
    SkPMColor4f fpPreModulation;
    for (int i = 0; i < 4; i++) {
        int modulationIndex = modulation_index(i, alphaModulation);
        if (in1f[modulationIndex] < 0.2f) {
            // Use the stepped frame
            fpPreModulation[i] = out2f[i] / in2f[modulationIndex];
        } else {
            fpPreModulation[i] = out1f[i] / in1f[modulationIndex];
        }
    }

    // With reconstructed pre-modulated FP output, derive the expected value of fp * input for each
    // of the transformed input colors.
    SkPMColor4f expected1 = alphaModulation ? (fpPreModulation * in1f.fA)
                                            : (fpPreModulation * in1f);
    SkPMColor4f expected2 = alphaModulation ? (fpPreModulation * in2f.fA)
                                            : (fpPreModulation * in2f);
    SkPMColor4f expected3 = alphaModulation ? (fpPreModulation * in3f.fA)
                                            : (fpPreModulation * in3f);

    return fuzzy_color_equals(out1f, expected1) &&
           fuzzy_color_equals(out2f, expected2) &&
           fuzzy_color_equals(out3f, expected3);
}

DEF_GPUTEST_FOR_GL_RENDERING_CONTEXTS(ProcessorOptimizationValidationTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    GrProxyProvider* proxyProvider = context->priv().proxyProvider();
    auto resourceProvider = context->priv().resourceProvider();
    using FPFactory = GrFragmentProcessorTestFactory;

    uint32_t seed = FLAGS_processorSeed;
    if (FLAGS_randomProcessorTest) {
        std::random_device rd;
        seed = rd();
    }
    // If a non-deterministic bot fails this test, check the output to see what seed it used, then
    // use --processorSeed <seed> (without --randomProcessorTest) to reproduce.
    SkRandom random(seed);

    const GrBackendFormat format =
            context->priv().caps()->getBackendFormatFromColorType(kRGBA_8888_SkColorType);

    // Make the destination context for the test.
    static constexpr int kRenderSize = 256;
    sk_sp<GrRenderTargetContext> rtc = context->priv().makeDeferredRenderTargetContext(
            format, SkBackingFit::kExact, kRenderSize, kRenderSize, kRGBA_8888_GrPixelConfig,
            GrColorType::kRGBA_8888, nullptr);

    sk_sp<GrTextureProxy> proxies[2];
    if (!init_test_textures(resourceProvider, proxyProvider, &random, proxies)) {
        ERRORF(reporter, "Could not create test textures");
        return;
    }
    GrProcessorTestData testData(&random, context, rtc.get(), proxies);

    // Coverage optimization uses three frames with a linearly transformed input texture.  The first
    // frame has no offset, second frames add .2 and .4, which should then be present as a fixed
    // difference between the frame outputs if the FP is properly following the modulation
    // requirements of the coverage optimization.
    static constexpr SkScalar kInputDelta = 0.2f;
    auto inputTexture1 = make_input_texture(proxyProvider, kRenderSize, kRenderSize, 0.0f);
    auto inputTexture2 = make_input_texture(proxyProvider, kRenderSize, kRenderSize, kInputDelta);
    auto inputTexture3 = make_input_texture(proxyProvider, kRenderSize, kRenderSize, 2*kInputDelta);

    // Encoded images are very verbose and this tests many potential images, so only export the
    // first failure (subsequent failures have a reasonable chance of being related).
    bool loggedFirstFailure = false;
    bool loggedFirstWarning = false;

    // Storage for the three frames required for coverage compatibility optimization. Each frame
    // uses the correspondingly numbered inputTextureX.
    std::unique_ptr<GrColor[]> readData1(new GrColor[kRenderSize * kRenderSize]);
    std::unique_ptr<GrColor[]> readData2(new GrColor[kRenderSize * kRenderSize]);
    std::unique_ptr<GrColor[]> readData3(new GrColor[kRenderSize * kRenderSize]);

    // Because processor factories configure themselves in random ways, this is not exhaustive.
    for (int i = 0; i < FPFactory::Count(); ++i) {
        int timesToInvokeFactory = 5;
        // Increase the number of attempts if the FP has child FPs since optimizations likely depend
        // on child optimizations being present.
        std::unique_ptr<GrFragmentProcessor> fp = FPFactory::MakeIdx(i, &testData);
        for (int j = 0; j < fp->numChildProcessors(); ++j) {
            // This value made a reasonable trade off between time and coverage when this test was
            // written.
            timesToInvokeFactory *= FPFactory::Count() / 2;
        }
#if defined(__MSVC_RUNTIME_CHECKS)
        // This test is infuriatingly slow with MSVC runtime checks enabled
        timesToInvokeFactory = 1;
#endif
        for (int j = 0; j < timesToInvokeFactory; ++j) {
            fp = FPFactory::MakeIdx(i, &testData);

            if (!fp->hasConstantOutputForConstantInput() && !fp->preservesOpaqueInput() &&
                !fp->compatibleWithCoverageAsAlpha()) {
                continue;
            }

            if (fp->compatibleWithCoverageAsAlpha()) {
                // 2nd and 3rd frames are only used when checking coverage optimization
                render_fp(context, rtc.get(), fp.get(), inputTexture2, readData2.get());
                render_fp(context, rtc.get(), fp.get(), inputTexture3, readData3.get());
            }
            // Draw base frame last so that rtc holds the original FP behavior if we need to
            // dump the image to the log.
            render_fp(context, rtc.get(), fp.get(), inputTexture1, readData1.get());

            if (0) {  // Useful to see what FPs are being tested.
                SkString children;
                for (int c = 0; c < fp->numChildProcessors(); ++c) {
                    if (!c) {
                        children.append("(");
                    }
                    children.append(fp->childProcessor(c).name());
                    children.append(c == fp->numChildProcessors() - 1 ? ")" : ", ");
                }
                SkDebugf("%s %s\n", fp->name(), children.c_str());
            }

            // This test has a history of being flaky on a number of devices. If an FP is logically
            // violating the optimizations, it's reasonable to expect it to violate requirements on
            // a large number of pixels in the image. Sporadic pixel violations are more indicative
            // of device errors and represents a separate problem.
#if defined(SK_BUILD_FOR_SKQP)
            static constexpr int kMaxAcceptableFailedPixels = 0; // Strict when running as SKQP
#else
            static constexpr int kMaxAcceptableFailedPixels = 2 * kRenderSize; // ~0.7% of the image
#endif

            int failedPixelCount = 0;
            // Collect first optimization failure message, to be output later as a warning or an
            // error depending on whether the rendering "passed" or failed.
            SkString coverageMessage;
            SkString opaqueMessage;
            SkString constMessage;
            for (int y = 0; y < kRenderSize; ++y) {
                for (int x = 0; x < kRenderSize; ++x) {
                    bool passing = true;
                    GrColor input = input_texel_color(x, y, 0.0f);
                    GrColor output = readData1.get()[y * kRenderSize + x];

                    if (fp->compatibleWithCoverageAsAlpha()) {
                        GrColor i2 = input_texel_color(x, y, kInputDelta);
                        GrColor i3 = input_texel_color(x, y, 2 * kInputDelta);

                        GrColor o2 = readData2.get()[y * kRenderSize + x];
                        GrColor o3 = readData3.get()[y * kRenderSize + x];

                        // A compatible processor is allowed to modulate either the input color or
                        // just the input alpha.
                        bool legalAlphaModulation = legal_modulation(input, i2, i3, output, o2, o3,
                                                                     /* alpha */ true);
                        bool legalColorModulation = legal_modulation(input, i2, i3, output, o2, o3,
                                                                     /* alpha */ false);

                        if (!legalColorModulation && !legalAlphaModulation) {
                            passing = false;

                            if (coverageMessage.isEmpty()) {
                                coverageMessage.printf("\"Modulating\" processor %s did not match "
                                        "alpha-modulation nor color-modulation rules. "
                                        "Input: 0x%08x, Output: 0x%08x, pixel (%d, %d).",
                                        fp->name(), input, output, x, y);
                            }
                        }
                    }

                    SkPMColor4f input4f = SkPMColor4f::FromBytes_RGBA(input);
                    SkPMColor4f output4f = SkPMColor4f::FromBytes_RGBA(output);
                    SkPMColor4f expected4f;
                    if (fp->hasConstantOutputForConstantInput(input4f, &expected4f)) {
                        float rDiff = fabsf(output4f.fR - expected4f.fR);
                        float gDiff = fabsf(output4f.fG - expected4f.fG);
                        float bDiff = fabsf(output4f.fB - expected4f.fB);
                        float aDiff = fabsf(output4f.fA - expected4f.fA);
                        static constexpr float kTol = 4 / 255.f;
                        if (rDiff > kTol || gDiff > kTol || bDiff > kTol || aDiff > kTol) {
                            if (constMessage.isEmpty()) {
                                passing = false;

                                constMessage.printf("Processor %s claimed output for const input "
                                        "doesn't match actual output. Error: %f, Tolerance: %f, "
                                        "input: (%f, %f, %f, %f), actual: (%f, %f, %f, %f), "
                                        "expected(%f, %f, %f, %f)", fp->name(),
                                        SkTMax(rDiff, SkTMax(gDiff, SkTMax(bDiff, aDiff))), kTol,
                                        input4f.fR, input4f.fG, input4f.fB, input4f.fA,
                                        output4f.fR, output4f.fG, output4f.fB, output4f.fA,
                                        expected4f.fR, expected4f.fG, expected4f.fB, expected4f.fA);
                            }
                        }
                    }
                    if (input4f.isOpaque() && fp->preservesOpaqueInput() && !output4f.isOpaque()) {
                        passing = false;

                        if (opaqueMessage.isEmpty()) {
                            opaqueMessage.printf("Processor %s claimed opaqueness is preserved but "
                                    "it is not. Input: 0x%08x, Output: 0x%08x.",
                                    fp->name(), input, output);
                        }
                    }

                    if (!passing) {
                        // Regardless of how many optimizations the pixel violates, count it as a
                        // single bad pixel.
                        failedPixelCount++;
                    }
                }
            }

            // Finished analyzing the entire image, see if the number of pixel failures meets the
            // threshold for an FP violating the optimization requirements.
            if (failedPixelCount > kMaxAcceptableFailedPixels) {
                ERRORF(reporter, "Processor violated %d of %d pixels, seed: 0x%08x, processor: %s"
                       ", first failing pixel details are below:",
                       failedPixelCount, kRenderSize * kRenderSize, seed,
                       fp->dumpInfo().c_str());

                // Print first failing pixel's details.
                if (!coverageMessage.isEmpty()) {
                    ERRORF(reporter, coverageMessage.c_str());
                }
                if (!constMessage.isEmpty()) {
                    ERRORF(reporter, constMessage.c_str());
                }
                if (!opaqueMessage.isEmpty()) {
                    ERRORF(reporter, opaqueMessage.c_str());
                }

                if (!loggedFirstFailure) {
                    // Print with ERRORF to make sure the encoded image is output
                    SkString input;
                    log_surface_proxy(context, inputTexture1, &input);
                    SkString output;
                    log_surface_context(rtc, &output);
                    ERRORF(reporter, "Input image: %s\n\n"
                           "===========================================================\n\n"
                           "Output image: %s\n", input.c_str(), output.c_str());
                    loggedFirstFailure = true;
                }
            } else if(failedPixelCount > 0) {
                // Don't trigger an error, but don't just hide the failures either.
                INFOF(reporter, "Processor violated %d of %d pixels (below error threshold), seed: "
                      "0x%08x, processor: %s", failedPixelCount, kRenderSize * kRenderSize,
                      seed, fp->dumpInfo().c_str());
                if (!coverageMessage.isEmpty()) {
                    INFOF(reporter, coverageMessage.c_str());
                }
                if (!constMessage.isEmpty()) {
                    INFOF(reporter, constMessage.c_str());
                }
                if (!opaqueMessage.isEmpty()) {
                    INFOF(reporter, opaqueMessage.c_str());
                }
                if (!loggedFirstWarning) {
                    SkString input;
                    log_surface_proxy(context, inputTexture1, &input);
                    SkString output;
                    log_surface_context(rtc, &output);
                    INFOF(reporter, "Input image: %s\n\n"
                          "===========================================================\n\n"
                          "Output image: %s\n", input.c_str(), output.c_str());
                    loggedFirstWarning = true;
                }
            }
        }
    }
}

// Tests that fragment processors returned by GrFragmentProcessor::clone() are equivalent to their
// progenitors.
DEF_GPUTEST_FOR_GL_RENDERING_CONTEXTS(ProcessorCloneTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    GrProxyProvider* proxyProvider = context->priv().proxyProvider();
    auto resourceProvider = context->priv().resourceProvider();

    SkRandom random;

    const GrBackendFormat format =
            context->priv().caps()->getBackendFormatFromColorType(kRGBA_8888_SkColorType);

    // Make the destination context for the test.
    static constexpr int kRenderSize = 1024;
    sk_sp<GrRenderTargetContext> rtc = context->priv().makeDeferredRenderTargetContext(
            format, SkBackingFit::kExact, kRenderSize, kRenderSize, kRGBA_8888_GrPixelConfig,
            GrColorType::kRGBA_8888, nullptr);

    sk_sp<GrTextureProxy> proxies[2];
    if (!init_test_textures(resourceProvider, proxyProvider, &random, proxies)) {
        ERRORF(reporter, "Could not create test textures");
        return;
    }
    GrProcessorTestData testData(&random, context, rtc.get(), proxies);

    auto inputTexture = make_input_texture(proxyProvider, kRenderSize, kRenderSize, 0.0f);
    std::unique_ptr<GrColor[]> readData1(new GrColor[kRenderSize * kRenderSize]);
    std::unique_ptr<GrColor[]> readData2(new GrColor[kRenderSize * kRenderSize]);
    auto readInfo = SkImageInfo::Make(kRenderSize, kRenderSize, kRGBA_8888_SkColorType,
                                      kPremul_SkAlphaType);

    // Because processor factories configure themselves in random ways, this is not exhaustive.
    for (int i = 0; i < GrFragmentProcessorTestFactory::Count(); ++i) {
        static constexpr int kTimesToInvokeFactory = 10;
        for (int j = 0; j < kTimesToInvokeFactory; ++j) {
            auto fp = GrFragmentProcessorTestFactory::MakeIdx(i, &testData);
            auto clone = fp->clone();
            if (!clone) {
                ERRORF(reporter, "Clone of processor %s failed.", fp->name());
                continue;
            }
            const char* name = fp->name();
            REPORTER_ASSERT(reporter, !strcmp(fp->name(), clone->name()));
            REPORTER_ASSERT(reporter, fp->compatibleWithCoverageAsAlpha() ==
                                      clone->compatibleWithCoverageAsAlpha());
            REPORTER_ASSERT(reporter, fp->isEqual(*clone));
            REPORTER_ASSERT(reporter, fp->preservesOpaqueInput() == clone->preservesOpaqueInput());
            REPORTER_ASSERT(reporter, fp->hasConstantOutputForConstantInput() ==
                                      clone->hasConstantOutputForConstantInput());
            REPORTER_ASSERT(reporter, fp->numChildProcessors() == clone->numChildProcessors());
            REPORTER_ASSERT(reporter, fp->usesLocalCoords() == clone->usesLocalCoords());
            // Draw with original and read back the results.
            render_fp(context, rtc.get(), fp.get(), inputTexture, readData1.get());

            // Draw with clone and read back the results.
            render_fp(context, rtc.get(), clone.get(), inputTexture, readData2.get());

            // Check that the results are the same.
            bool passing = true;
            for (int y = 0; y < kRenderSize && passing; ++y) {
                for (int x = 0; x < kRenderSize && passing; ++x) {
                    int idx = y * kRenderSize + x;
                    if (readData1[idx] != readData2[idx]) {
                        ERRORF(reporter,
                               "Processor %s made clone produced different output. "
                               "Input color: 0x%08x, Original Output Color: 0x%08x, "
                               "Clone Output Color: 0x%08x..",
                               name, input_texel_color(x, y, 0.0f), readData1[idx], readData2[idx]);
                        passing = false;
                    }
                }
            }
        }
    }
}

#endif  // GR_TEST_UTILS
