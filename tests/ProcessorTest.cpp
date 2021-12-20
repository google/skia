/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/gpu/GrDirectContext.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrGpuResource.h"
#include "src/gpu/GrImageInfo.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/KeyBuilder.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrTextureEffect.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/ops/GrMeshDrawOp.h"
#include "src/gpu/v1/SurfaceDrawContext_v1.h"
#include "tests/TestUtils.h"

#include <atomic>
#include <random>

namespace {
class TestOp : public GrMeshDrawOp {
public:
    DEFINE_OP_CLASS_ID
    static GrOp::Owner Make(GrRecordingContext* rContext,
                            std::unique_ptr<GrFragmentProcessor> fp) {
        return GrOp::Make<TestOp>(rContext, std::move(fp));
    }

    const char* name() const override { return "TestOp"; }

    void visitProxies(const GrVisitProxyFunc& func) const override {
        fProcessors.visitProxies(func);
    }

    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }

    GrProcessorSet::Analysis finalize(const GrCaps& caps, const GrAppliedClip* clip,
                                      GrClampType clampType) override {
        static constexpr GrProcessorAnalysisColor kUnknownColor;
        SkPMColor4f overrideColor;
        return fProcessors.finalize(
                kUnknownColor, GrProcessorAnalysisCoverage::kNone, clip,
                &GrUserStencilSettings::kUnused, caps, clampType, &overrideColor);
    }

private:
    friend class ::GrOp; // for ctor

    TestOp(std::unique_ptr<GrFragmentProcessor> fp)
            : INHERITED(ClassID()), fProcessors(std::move(fp)) {
        this->setBounds(SkRect::MakeWH(100, 100), HasAABloat::kNo, IsHairline::kNo);
    }

    GrProgramInfo* programInfo() override { return nullptr; }
    void onCreateProgramInfo(const GrCaps*,
                             SkArenaAlloc*,
                             const GrSurfaceProxyView& writeView,
                             bool usesMSAASurface,
                             GrAppliedClip&&,
                             const GrDstProxyView&,
                             GrXferBarrierFlags renderPassXferBarriers,
                             GrLoadOp colorLoadOp) override {}
    void onPrePrepareDraws(GrRecordingContext*,
                           const GrSurfaceProxyView& writeView,
                           GrAppliedClip*,
                           const GrDstProxyView&,
                           GrXferBarrierFlags renderPassXferBarriers,
                           GrLoadOp colorLoadOp) override {}
    void onPrepareDraws(GrMeshDrawTarget*) override { return; }
    void onExecute(GrOpFlushState*, const SkRect&) override { return; }

    GrProcessorSet fProcessors;

    using INHERITED = GrMeshDrawOp;
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
    static std::unique_ptr<GrFragmentProcessor> Make(const SkTArray<GrSurfaceProxyView>& views) {
        return std::unique_ptr<GrFragmentProcessor>(new TestFP(views));
    }

    const char* name() const override { return "test"; }

    void onAddToKey(const GrShaderCaps&, skgpu::KeyBuilder* b) const override {
        static std::atomic<int32_t> nextKey{0};
        b->add32(nextKey++);
    }

    std::unique_ptr<GrFragmentProcessor> clone() const override {
        return std::unique_ptr<GrFragmentProcessor>(new TestFP(*this));
    }

private:
    TestFP(const SkTArray<GrSurfaceProxyView>& views)
            : INHERITED(kTestFP_ClassID, kNone_OptimizationFlags) {
        for (const GrSurfaceProxyView& view : views) {
            this->registerChild(GrTextureEffect::Make(view, kUnknown_SkAlphaType));
        }
    }

    TestFP(std::unique_ptr<GrFragmentProcessor> child)
            : INHERITED(kTestFP_ClassID, kNone_OptimizationFlags) {
        this->registerChild(std::move(child));
    }

    explicit TestFP(const TestFP& that) : INHERITED(that) {}

    std::unique_ptr<ProgramImpl> onMakeProgramImpl() const override {
        class Impl : public ProgramImpl {
        public:
            void emitCode(EmitArgs& args) override {
                args.fFragBuilder->codeAppendf("return half4(1);");
            }

        private:
        };
        return std::make_unique<Impl>();
    }

    bool onIsEqual(const GrFragmentProcessor&) const override { return false; }

    using INHERITED = GrFragmentProcessor;
};
}  // namespace

DEF_GPUTEST_FOR_ALL_CONTEXTS(ProcessorRefTest, reporter, ctxInfo) {
    auto dContext = ctxInfo.directContext();
    GrProxyProvider* proxyProvider = dContext->priv().proxyProvider();

    static constexpr SkISize kDims = {10, 10};

    const GrBackendFormat format =
        dContext->priv().caps()->getDefaultBackendFormat(GrColorType::kRGBA_8888,
                                                         GrRenderable::kNo);
    GrSwizzle swizzle = dContext->priv().caps()->getReadSwizzle(format, GrColorType::kRGBA_8888);

    for (bool makeClone : {false, true}) {
        for (int parentCnt = 0; parentCnt < 2; parentCnt++) {
            auto sdc = skgpu::v1::SurfaceDrawContext::Make(
                    dContext, GrColorType::kRGBA_8888, nullptr, SkBackingFit::kApprox, {1, 1},
                    SkSurfaceProps());
            {
                sk_sp<GrTextureProxy> proxy = proxyProvider->createProxy(
                        format, kDims, GrRenderable::kNo, 1, GrMipmapped::kNo, SkBackingFit::kExact,
                        SkBudgeted::kYes, GrProtected::kNo);

                {
                    SkTArray<GrSurfaceProxyView> views;
                    views.push_back({proxy, kTopLeft_GrSurfaceOrigin, swizzle});
                    auto fp = TestFP::Make(std::move(views));
                    for (int i = 0; i < parentCnt; ++i) {
                        fp = TestFP::Make(std::move(fp));
                    }
                    std::unique_ptr<GrFragmentProcessor> clone;
                    if (makeClone) {
                        clone = fp->clone();
                    }
                    GrOp::Owner op = TestOp::Make(dContext, std::move(fp));
                    sdc->addDrawOp(std::move(op));
                    if (clone) {
                        op = TestOp::Make(dContext, std::move(clone));
                        sdc->addDrawOp(std::move(op));
                    }
                }

                // If the fp is cloned the number of refs should increase by one (for the clone)
                int expectedProxyRefs = makeClone ? 3 : 2;

                CheckSingleThreadedProxyRefs(reporter, proxy.get(), expectedProxyRefs, -1);

                dContext->flushAndSubmit();

                // just one from the 'proxy' sk_sp
                CheckSingleThreadedProxyRefs(reporter, proxy.get(), 1, 1);
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

static GrColor input_texel_color(int x, int y, SkScalar delta) {
    // Delta must be less than 0.5 to prevent over/underflow issues with the input color
    SkASSERT(delta <= 0.5);

    SkColor color = SkColorSetARGB((uint8_t)(x & 0xFF),
                                   (uint8_t)(y & 0xFF),
                                   (uint8_t)((x + y) & 0xFF),
                                   (uint8_t)((2 * y - x) & 0xFF));
    SkColor4f color4f = SkColor4f::FromColor(color);
    // We only apply delta to the r,g, and b channels. This is because we're using this
    // to test the canTweakAlphaForCoverage() optimization. A processor is allowed
    // to use the input color's alpha in its calculation and report this optimization.
    for (int i = 0; i < 3; i++) {
        if (color4f[i] > 0.5) {
            color4f[i] -= delta;
        } else {
            color4f[i] += delta;
        }
    }
    return color4f.premul().toBytes_RGBA();
}

// The output buffer must be the same size as the render-target context.
static void render_fp(GrDirectContext* dContext,
                      skgpu::v1::SurfaceDrawContext* sdc,
                      std::unique_ptr<GrFragmentProcessor> fp,
                      GrColor* outBuffer) {
    sdc->fillWithFP(std::move(fp));
    std::fill_n(outBuffer, sdc->width() * sdc->height(), 0);
    auto ii = SkImageInfo::Make(sdc->dimensions(), kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    GrPixmap resultPM(ii, outBuffer, sdc->width()*sizeof(uint32_t));
    sdc->readPixels(dContext, resultPM, {0, 0});
}

// This class is responsible for reproducibly generating a random fragment processor.
// An identical randomly-designed FP can be generated as many times as needed.
class TestFPGenerator {
    public:
        TestFPGenerator() = delete;
        TestFPGenerator(GrDirectContext* context, GrResourceProvider* resourceProvider)
                : fContext(context)
                , fResourceProvider(resourceProvider)
                , fInitialSeed(synthesizeInitialSeed())
                , fRandomSeed(fInitialSeed) {}

        uint32_t initialSeed() { return fInitialSeed; }

        bool init() {
            // Initializes the two test texture proxies that are available to the FP test factories.
            SkRandom random{fRandomSeed};
            static constexpr int kTestTextureSize = 256;

            {
                // Put premul data into the RGBA texture that the test FPs can optionally use.
                GrColor* rgbaData = new GrColor[kTestTextureSize * kTestTextureSize];
                for (int y = 0; y < kTestTextureSize; ++y) {
                    for (int x = 0; x < kTestTextureSize; ++x) {
                        rgbaData[kTestTextureSize * y + x] = input_texel_color(
                                random.nextULessThan(256), random.nextULessThan(256), 0.0f);
                    }
                }

                SkImageInfo ii = SkImageInfo::Make(kTestTextureSize, kTestTextureSize,
                                                   kRGBA_8888_SkColorType, kPremul_SkAlphaType);
                SkBitmap bitmap;
                bitmap.installPixels(
                        ii, rgbaData, ii.minRowBytes(),
                        [](void* addr, void* context) { delete[](GrColor*) addr; }, nullptr);
                bitmap.setImmutable();
                auto view = std::get<0>(GrMakeUncachedBitmapProxyView(fContext, bitmap));
                if (!view || !view.proxy()->instantiate(fResourceProvider)) {
                    SkDebugf("Unable to instantiate RGBA8888 test texture.");
                    return false;
                }
                fTestViews[0] = GrProcessorTestData::ViewInfo{view, GrColorType::kRGBA_8888,
                                                              kPremul_SkAlphaType};
            }

            {
                // Put random values into the alpha texture that the test FPs can optionally use.
                uint8_t* alphaData = new uint8_t[kTestTextureSize * kTestTextureSize];
                for (int y = 0; y < kTestTextureSize; ++y) {
                    for (int x = 0; x < kTestTextureSize; ++x) {
                        alphaData[kTestTextureSize * y + x] = random.nextULessThan(256);
                    }
                }

                SkImageInfo ii = SkImageInfo::Make(kTestTextureSize, kTestTextureSize,
                                                   kAlpha_8_SkColorType, kPremul_SkAlphaType);
                SkBitmap bitmap;
                bitmap.installPixels(
                        ii, alphaData, ii.minRowBytes(),
                        [](void* addr, void* context) { delete[](uint8_t*) addr; }, nullptr);
                bitmap.setImmutable();
                auto view = std::get<0>(GrMakeUncachedBitmapProxyView(fContext, bitmap));
                if (!view || !view.proxy()->instantiate(fResourceProvider)) {
                    SkDebugf("Unable to instantiate A8 test texture.");
                    return false;
                }
                fTestViews[1] = GrProcessorTestData::ViewInfo{view, GrColorType::kAlpha_8,
                                                              kPremul_SkAlphaType};
            }

            return true;
        }

        void reroll() {
            // Feed our current random seed into SkRandom to generate a new seed.
            SkRandom random{fRandomSeed};
            fRandomSeed = random.nextU();
        }

        std::unique_ptr<GrFragmentProcessor> make(int type, int randomTreeDepth,
                                                  std::unique_ptr<GrFragmentProcessor> inputFP) {
            // This will generate the exact same randomized FP (of each requested type) each time
            // it's called. Call `reroll` to get a different FP.
            SkRandom random{fRandomSeed};
            GrProcessorTestData testData{&random, fContext, randomTreeDepth,
                                         SK_ARRAY_COUNT(fTestViews), fTestViews,
                                         std::move(inputFP)};
            return GrFragmentProcessorTestFactory::MakeIdx(type, &testData);
        }

        std::unique_ptr<GrFragmentProcessor> make(int type, int randomTreeDepth,
                                                  GrSurfaceProxyView view,
                                                  SkAlphaType alpha = kPremul_SkAlphaType) {
            return make(type, randomTreeDepth, GrTextureEffect::Make(view, alpha));
        }

    private:
        static uint32_t synthesizeInitialSeed() {
            if (FLAGS_randomProcessorTest) {
                std::random_device rd;
                return rd();
            } else {
                return FLAGS_processorSeed;
            }
        }

        GrDirectContext* fContext;              // owned by caller
        GrResourceProvider* fResourceProvider;  // owned by caller
        const uint32_t fInitialSeed;
        uint32_t fRandomSeed;
        GrProcessorTestData::ViewInfo fTestViews[2];
};

// Creates an array of color values from input_texel_color(), to be used as an input texture.
static std::vector<GrColor> make_input_pixels(int width, int height, SkScalar delta) {
    std::vector<GrColor> pixel(width * height);
    for (int y = 0; y < width; ++y) {
        for (int x = 0; x < height; ++x) {
            pixel[width * y + x] = input_texel_color(x, y, delta);
        }
    }

    return pixel;
}

// Creates a texture of premul colors used as the output of the fragment processor that precedes
// the fragment processor under test. An array of W*H colors are passed in as the texture data.
static GrSurfaceProxyView make_input_texture(GrRecordingContext* context,
                                      int width, int height, GrColor* pixel) {
    SkImageInfo ii = SkImageInfo::Make(width, height, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    SkBitmap bitmap;
    bitmap.installPixels(ii, pixel, ii.minRowBytes());
    bitmap.setImmutable();
    return std::get<0>(GrMakeUncachedBitmapProxyView(context, bitmap));
}

// We tag logged data as unpremul to avoid conversion when encoding as PNG. The input texture
// actually contains unpremul data. Also, even though we made the result data by rendering into
// a "unpremul" SurfaceDrawContext, our input texture is unpremul and outside of the random
// effect configuration, we didn't do anything to ensure the output is actually premul. We just
// don't currently allow kUnpremul GrSurfaceDrawContexts.
static constexpr auto kLogAlphaType = kUnpremul_SkAlphaType;

static bool log_pixels(GrColor* pixels, int widthHeight, SkString* dst) {
    SkImageInfo info =
            SkImageInfo::Make(widthHeight, widthHeight, kRGBA_8888_SkColorType, kLogAlphaType);
    SkBitmap bmp;
    bmp.installPixels(info, pixels, widthHeight * sizeof(GrColor));
    return BipmapToBase64DataURI(bmp, dst);
}

static bool log_texture_view(GrDirectContext* dContext, GrSurfaceProxyView src, SkString* dst) {
    SkImageInfo ii = SkImageInfo::Make(src.proxy()->dimensions(), kRGBA_8888_SkColorType,
                                       kLogAlphaType);

    auto sContext = dContext->priv().makeSC(std::move(src), ii.colorInfo());
    SkBitmap bm;
    SkAssertResult(bm.tryAllocPixels(ii));
    SkAssertResult(sContext->readPixels(dContext, bm.pixmap(), {0, 0}));
    return BipmapToBase64DataURI(bm, dst);
}

static bool fuzzy_color_equals(const SkPMColor4f& c1, const SkPMColor4f& c2) {
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

// Given three input colors (color preceding the FP being tested) provided to the FP at the same
// local coord and the three corresponding FP outputs, this ensures that either:
//   out[0] = fp * in[0].a, out[1] = fp * in[1].a, and out[2] = fp * in[2].a
// where fp is the pre-modulated color that should not be changing across frames (FP's state doesn't
// change), OR:
//   out[0] = fp * in[0], out[1] = fp * in[1], and out[2] = fp * in[2]
// (per-channel modulation instead of modulation by just the alpha channel)
// It does this by estimating the pre-modulated fp color from one of the input/output pairs and
// confirms the conditions hold for the other two pairs.
// It is required that the three input colors have the same alpha as fp is allowed to be a function
// of the input alpha (but not r, g, or b).
static bool legal_modulation(const GrColor inGr[3], const GrColor outGr[3]) {
    // Convert to floating point, which is the number space the FP operates in (more or less)
    SkPMColor4f inf[3], outf[3];
    for (int i = 0; i < 3; ++i) {
        inf[i]  = SkPMColor4f::FromBytes_RGBA(inGr[i]);
        outf[i] = SkPMColor4f::FromBytes_RGBA(outGr[i]);
    }
    // This test is only valid if all the input alphas are the same.
    SkASSERT(inf[0].fA == inf[1].fA && inf[1].fA == inf[2].fA);

    // Reconstruct the output of the FP before the shader modulated its color with the input value.
    // When the original input is very small, it may cause the final output color to round
    // to 0, in which case we estimate the pre-modulated color using one of the stepped frames that
    // will then have a guaranteed larger channel value (since the offset will be added to it).
    SkPMColor4f fpPreColorModulation = {0,0,0,0};
    SkPMColor4f fpPreAlphaModulation = {0,0,0,0};
    for (int i = 0; i < 4; i++) {
        // Use the most stepped up frame
        int maxInIdx = inf[0][i] > inf[1][i] ? 0 : 1;
        maxInIdx = inf[maxInIdx][i] > inf[2][i] ? maxInIdx : 2;
        const SkPMColor4f& in = inf[maxInIdx];
        const SkPMColor4f& out = outf[maxInIdx];
        if (in[i] > 0) {
            fpPreColorModulation[i] = out[i] / in[i];
        }
        if (in[3] > 0) {
            fpPreAlphaModulation[i] = out[i] / in[3];
        }
    }

    // With reconstructed pre-modulated FP output, derive the expected value of fp * input for each
    // of the transformed input colors.
    SkPMColor4f expectedForAlphaModulation[3];
    SkPMColor4f expectedForColorModulation[3];
    for (int i = 0; i < 3; ++i) {
        expectedForAlphaModulation[i] = fpPreAlphaModulation * inf[i].fA;
        expectedForColorModulation[i] = fpPreColorModulation * inf[i];
        // If the input alpha is 0 then the other channels should also be zero
        // since the color is assumed to be premul. Modulating zeros by anything
        // should produce zeros.
        if (inf[i].fA == 0) {
            SkASSERT(inf[i].fR == 0 && inf[i].fG == 0 && inf[i].fB == 0);
            expectedForColorModulation[i] = expectedForAlphaModulation[i] = {0, 0, 0, 0};
        }
    }

    bool isLegalColorModulation = fuzzy_color_equals(outf[0], expectedForColorModulation[0]) &&
                                  fuzzy_color_equals(outf[1], expectedForColorModulation[1]) &&
                                  fuzzy_color_equals(outf[2], expectedForColorModulation[2]);

    bool isLegalAlphaModulation = fuzzy_color_equals(outf[0], expectedForAlphaModulation[0]) &&
                                  fuzzy_color_equals(outf[1], expectedForAlphaModulation[1]) &&
                                  fuzzy_color_equals(outf[2], expectedForAlphaModulation[2]);

    // This can be enabled to print the values that caused this check to fail.
    if (0 && !isLegalColorModulation && !isLegalAlphaModulation) {
        SkDebugf("Color modulation test\n\timplied mod color: (%.03f, %.03f, %.03f, %.03f)\n",
                 fpPreColorModulation[0],
                 fpPreColorModulation[1],
                 fpPreColorModulation[2],
                 fpPreColorModulation[3]);
        for (int i = 0; i < 3; ++i) {
            SkDebugf("\t(%.03f, %.03f, %.03f, %.03f) -> "
                     "(%.03f, %.03f, %.03f, %.03f) | "
                     "(%.03f, %.03f, %.03f, %.03f), ok: %d\n",
                     inf[i].fR, inf[i].fG, inf[i].fB, inf[i].fA,
                     outf[i].fR, outf[i].fG, outf[i].fB, outf[i].fA,
                     expectedForColorModulation[i].fR, expectedForColorModulation[i].fG,
                     expectedForColorModulation[i].fB, expectedForColorModulation[i].fA,
                     fuzzy_color_equals(outf[i], expectedForColorModulation[i]));
        }
        SkDebugf("Alpha modulation test\n\timplied mod color: (%.03f, %.03f, %.03f, %.03f)\n",
                 fpPreAlphaModulation[0],
                 fpPreAlphaModulation[1],
                 fpPreAlphaModulation[2],
                 fpPreAlphaModulation[3]);
        for (int i = 0; i < 3; ++i) {
            SkDebugf("\t(%.03f, %.03f, %.03f, %.03f) -> "
                     "(%.03f, %.03f, %.03f, %.03f) | "
                     "(%.03f, %.03f, %.03f, %.03f), ok: %d\n",
                     inf[i].fR, inf[i].fG, inf[i].fB, inf[i].fA,
                     outf[i].fR, outf[i].fG, outf[i].fB, outf[i].fA,
                     expectedForAlphaModulation[i].fR, expectedForAlphaModulation[i].fG,
                     expectedForAlphaModulation[i].fB, expectedForAlphaModulation[i].fA,
                     fuzzy_color_equals(outf[i], expectedForAlphaModulation[i]));
        }
    }
    return isLegalColorModulation || isLegalAlphaModulation;
}

DEF_GPUTEST_FOR_GL_RENDERING_CONTEXTS(ProcessorOptimizationValidationTest, reporter, ctxInfo) {
    GrDirectContext* context = ctxInfo.directContext();
    GrResourceProvider* resourceProvider = context->priv().resourceProvider();
    using FPFactory = GrFragmentProcessorTestFactory;

    TestFPGenerator fpGenerator{context, resourceProvider};
    if (!fpGenerator.init()) {
        ERRORF(reporter, "Could not initialize TestFPGenerator");
        return;
    }

    // Make the destination context for the test.
    static constexpr int kRenderSize = 256;
    auto sdc = skgpu::v1::SurfaceDrawContext::Make(
            context, GrColorType::kRGBA_8888, nullptr, SkBackingFit::kExact,
            {kRenderSize, kRenderSize}, SkSurfaceProps());

    // Coverage optimization uses three frames with a linearly transformed input texture.  The first
    // frame has no offset, second frames add .2 and .4, which should then be present as a fixed
    // difference between the frame outputs if the FP is properly following the modulation
    // requirements of the coverage optimization.
    static constexpr SkScalar kInputDelta = 0.2f;
    std::vector<GrColor> inputPixels1 = make_input_pixels(kRenderSize, kRenderSize, 0.0f);
    std::vector<GrColor> inputPixels2 =
            make_input_pixels(kRenderSize, kRenderSize, 1 * kInputDelta);
    std::vector<GrColor> inputPixels3 =
            make_input_pixels(kRenderSize, kRenderSize, 2 * kInputDelta);
    GrSurfaceProxyView inputTexture1 =
            make_input_texture(context, kRenderSize, kRenderSize, inputPixels1.data());
    GrSurfaceProxyView inputTexture2 =
            make_input_texture(context, kRenderSize, kRenderSize, inputPixels2.data());
    GrSurfaceProxyView inputTexture3 =
            make_input_texture(context, kRenderSize, kRenderSize, inputPixels3.data());

    // Encoded images are very verbose and this tests many potential images, so only export the
    // first failure (subsequent failures have a reasonable chance of being related).
    bool loggedFirstFailure = false;
    bool loggedFirstWarning = false;

    // Storage for the three frames required for coverage compatibility optimization testing.
    // Each frame uses the correspondingly numbered inputTextureX.
    std::vector<GrColor> readData1(kRenderSize * kRenderSize);
    std::vector<GrColor> readData2(kRenderSize * kRenderSize);
    std::vector<GrColor> readData3(kRenderSize * kRenderSize);

    // Because processor factories configure themselves in random ways, this is not exhaustive.
    for (int i = 0; i < FPFactory::Count(); ++i) {
        int optimizedForOpaqueInput = 0;
        int optimizedForCoverageAsAlpha = 0;
        int optimizedForConstantOutputForInput = 0;

#ifdef __MSVC_RUNTIME_CHECKS
        // This test is infuriatingly slow with MSVC runtime checks enabled
        static constexpr int kMinimumTrials = 1;
        static constexpr int kMaximumTrials = 1;
        static constexpr int kExpectedSuccesses = 1;
#else
        // We start by testing each fragment-processor 100 times, watching the optimization bits
        // that appear. If we see an optimization bit appear in those first 100 trials, we keep
        // running tests until we see at least five successful trials that have this optimization
        // bit enabled. If we never see a particular optimization bit after 100 trials, we assume
        // that this FP doesn't support that optimization at all.
        static constexpr int kMinimumTrials = 100;
        static constexpr int kMaximumTrials = 2000;
        static constexpr int kExpectedSuccesses = 5;
#endif

        for (int trial = 0;; ++trial) {
            // Create a randomly-configured FP.
            fpGenerator.reroll();
            std::unique_ptr<GrFragmentProcessor> fp =
                    fpGenerator.make(i, /*randomTreeDepth=*/1, inputTexture1);

            // If we have iterated enough times and seen a sufficient number of successes on each
            // optimization bit that can be returned, stop running trials.
            if (trial >= kMinimumTrials) {
                bool moreTrialsNeeded = (optimizedForOpaqueInput > 0 &&
                                         optimizedForOpaqueInput < kExpectedSuccesses) ||
                                        (optimizedForCoverageAsAlpha > 0 &&
                                         optimizedForCoverageAsAlpha < kExpectedSuccesses) ||
                                        (optimizedForConstantOutputForInput > 0 &&
                                         optimizedForConstantOutputForInput < kExpectedSuccesses);
                if (!moreTrialsNeeded) break;

                if (trial >= kMaximumTrials) {
                    SkDebugf("Abandoning ProcessorOptimizationValidationTest after %d trials. "
                             "Seed: 0x%08x, processor:\n%s",
                             kMaximumTrials, fpGenerator.initialSeed(), fp->dumpTreeInfo().c_str());
                    break;
                }
            }

            // Skip further testing if this trial has no optimization bits enabled.
            if (!fp->hasConstantOutputForConstantInput() && !fp->preservesOpaqueInput() &&
                !fp->compatibleWithCoverageAsAlpha()) {
                continue;
            }

            // We can make identical copies of the test FP in order to test coverage-as-alpha.
            if (fp->compatibleWithCoverageAsAlpha()) {
                // Create and render two identical versions of this FP, but using different input
                // textures, to check coverage optimization. We don't need to do this step for
                // constant-output or preserving-opacity tests.
                render_fp(context, sdc.get(),
                          fpGenerator.make(i, /*randomTreeDepth=*/1, inputTexture2),
                          readData2.data());
                render_fp(context, sdc.get(),
                          fpGenerator.make(i, /*randomTreeDepth=*/1, inputTexture3),
                          readData3.data());
                ++optimizedForCoverageAsAlpha;
            }

            if (fp->hasConstantOutputForConstantInput()) {
                ++optimizedForConstantOutputForInput;
            }

            if (fp->preservesOpaqueInput()) {
                ++optimizedForOpaqueInput;
            }

            // Draw base frame last so that rtc holds the original FP behavior if we need to dump
            // the image to the log.
            render_fp(context, sdc.get(), fpGenerator.make(i, /*randomTreeDepth=*/1, inputTexture1),
                      readData1.data());

            // This test has a history of being flaky on a number of devices. If an FP is logically
            // violating the optimizations, it's reasonable to expect it to violate requirements on
            // a large number of pixels in the image. Sporadic pixel violations are more indicative
            // of device errors and represents a separate problem.
#if defined(SK_BUILD_FOR_SKQP)
            static constexpr int kMaxAcceptableFailedPixels = 0; // Strict when running as SKQP
#else
            static constexpr int kMaxAcceptableFailedPixels = 2 * kRenderSize; // ~0.7% of the image
#endif

            // Collect first optimization failure message, to be output later as a warning or an
            // error depending on whether the rendering "passed" or failed.
            int failedPixelCount = 0;
            SkString coverageMessage;
            SkString opaqueMessage;
            SkString constMessage;
            for (int y = 0; y < kRenderSize; ++y) {
                for (int x = 0; x < kRenderSize; ++x) {
                    bool passing = true;
                    GrColor input = inputPixels1[y * kRenderSize + x];
                    GrColor output = readData1[y * kRenderSize + x];

                    if (fp->compatibleWithCoverageAsAlpha()) {
                        GrColor ins[3];
                        ins[0] = input;
                        ins[1] = inputPixels2[y * kRenderSize + x];
                        ins[2] = inputPixels3[y * kRenderSize + x];

                        GrColor outs[3];
                        outs[0] = output;
                        outs[1] = readData2[y * kRenderSize + x];
                        outs[2] = readData3[y * kRenderSize + x];

                        if (!legal_modulation(ins, outs)) {
                            passing = false;
                            if (coverageMessage.isEmpty()) {
                                coverageMessage.printf(
                                        "\"Modulating\" processor did not match alpha-modulation "
                                        "nor color-modulation rules.\n"
                                        "Input: 0x%08x, Output: 0x%08x, pixel (%d, %d).",
                                        input, output, x, y);
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

                                constMessage.printf(
                                        "Processor claimed output for const input doesn't match "
                                        "actual output.\n"
                                        "Error: %f, Tolerance: %f, input: (%f, %f, %f, %f), "
                                        "actual: (%f, %f, %f, %f), expected(%f, %f, %f, %f).",
                                        std::max(rDiff, std::max(gDiff, std::max(bDiff, aDiff))),
                                        kTol, input4f.fR, input4f.fG, input4f.fB, input4f.fA,
                                        output4f.fR, output4f.fG, output4f.fB, output4f.fA,
                                        expected4f.fR, expected4f.fG, expected4f.fB, expected4f.fA);
                            }
                        }
                    }
                    if (input4f.isOpaque() && fp->preservesOpaqueInput() && !output4f.isOpaque()) {
                        passing = false;

                        if (opaqueMessage.isEmpty()) {
                            opaqueMessage.printf(
                                    "Processor claimed opaqueness is preserved but "
                                    "it is not. Input: 0x%08x, Output: 0x%08x.",
                                    input, output);
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
                ERRORF(reporter,
                       "Processor violated %d of %d pixels, seed: 0x%08x.\n"
                       "Processor:\n%s\nFirst failing pixel details are below:",
                       failedPixelCount, kRenderSize * kRenderSize, fpGenerator.initialSeed(),
                       fp->dumpTreeInfo().c_str());

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
                    log_texture_view(context, inputTexture1, &input);
                    SkString output;
                    log_pixels(readData1.data(), kRenderSize, &output);
                    ERRORF(reporter, "Input image: %s\n\n"
                           "===========================================================\n\n"
                           "Output image: %s\n", input.c_str(), output.c_str());
                    loggedFirstFailure = true;
                }
            } else if (failedPixelCount > 0) {
                // Don't trigger an error, but don't just hide the failures either.
                INFOF(reporter, "Processor violated %d of %d pixels (below error threshold), seed: "
                      "0x%08x, processor: %s", failedPixelCount, kRenderSize * kRenderSize,
                      fpGenerator.initialSeed(), fp->dumpInfo().c_str());
                if (!coverageMessage.isEmpty()) {
                    INFOF(reporter, "%s", coverageMessage.c_str());
                }
                if (!constMessage.isEmpty()) {
                    INFOF(reporter, "%s", constMessage.c_str());
                }
                if (!opaqueMessage.isEmpty()) {
                    INFOF(reporter, "%s", opaqueMessage.c_str());
                }
                if (!loggedFirstWarning) {
                    SkString input;
                    log_texture_view(context, inputTexture1, &input);
                    SkString output;
                    log_pixels(readData1.data(), kRenderSize, &output);
                    INFOF(reporter, "Input image: %s\n\n"
                          "===========================================================\n\n"
                          "Output image: %s\n", input.c_str(), output.c_str());
                    loggedFirstWarning = true;
                }
            }
        }
    }
}

static void assert_processor_equality(skiatest::Reporter* reporter,
                                      const GrFragmentProcessor& fp,
                                      const GrFragmentProcessor& clone) {
    REPORTER_ASSERT(reporter, !strcmp(fp.name(), clone.name()),
                              "\n%s", fp.dumpTreeInfo().c_str());
    REPORTER_ASSERT(reporter, fp.compatibleWithCoverageAsAlpha() ==
                              clone.compatibleWithCoverageAsAlpha(),
                              "\n%s", fp.dumpTreeInfo().c_str());
    REPORTER_ASSERT(reporter, fp.isEqual(clone),
                              "\n%s", fp.dumpTreeInfo().c_str());
    REPORTER_ASSERT(reporter, fp.preservesOpaqueInput() == clone.preservesOpaqueInput(),
                              "\n%s", fp.dumpTreeInfo().c_str());
    REPORTER_ASSERT(reporter, fp.hasConstantOutputForConstantInput() ==
                              clone.hasConstantOutputForConstantInput(),
                              "\n%s", fp.dumpTreeInfo().c_str());
    REPORTER_ASSERT(reporter, fp.numChildProcessors() == clone.numChildProcessors(),
                              "\n%s", fp.dumpTreeInfo().c_str());
    REPORTER_ASSERT(reporter, fp.sampleUsage() == clone.sampleUsage(),
                              "\n%s", fp.dumpTreeInfo().c_str());
    REPORTER_ASSERT(reporter, fp.usesSampleCoords() == clone.usesSampleCoords(),
                              "\n%s", fp.dumpTreeInfo().c_str());
}

static bool verify_identical_render(skiatest::Reporter* reporter, int renderSize,
                                    const char* processorType,
                                    const GrColor readData1[], const GrColor readData2[]) {
    // The ProcessorClone test has a history of being flaky on a number of devices. If an FP clone
    // is logically wrong, it's reasonable to expect it produce a large number of pixel differences
    // in the image. Sporadic pixel violations are more indicative device errors and represents a
    // separate problem.
#if defined(SK_BUILD_FOR_SKQP)
    const int maxAcceptableFailedPixels = 0;  // Strict when running as SKQP
#else
    const int maxAcceptableFailedPixels = 2 * renderSize;  // ~0.002% of the pixels (size 1024*1024)
#endif

    int failedPixelCount = 0;
    int firstWrongX = 0;
    int firstWrongY = 0;
    int idx = 0;
    for (int y = 0; y < renderSize; ++y) {
        for (int x = 0; x < renderSize; ++x, ++idx) {
            if (readData1[idx] != readData2[idx]) {
                if (!failedPixelCount) {
                    firstWrongX = x;
                    firstWrongY = y;
                }
                ++failedPixelCount;
            }
            if (failedPixelCount > maxAcceptableFailedPixels) {
                idx = firstWrongY * renderSize + firstWrongX;
                ERRORF(reporter,
                       "%s produced different output at (%d, %d). "
                       "Input color: 0x%08x, Original Output Color: 0x%08x, "
                       "Clone Output Color: 0x%08x.",
                       processorType, firstWrongX, firstWrongY, input_texel_color(x, y, 0.0f),
                       readData1[idx], readData2[idx]);

                return false;
            }
        }
    }

    return true;
}

static void log_clone_failure(skiatest::Reporter* reporter, int renderSize,
                              GrDirectContext* context, const GrSurfaceProxyView& inputTexture,
                              GrColor pixelsFP[], GrColor pixelsClone[], GrColor pixelsRegen[]) {
    // Write the images out as data URLs for inspection.
    SkString inputURL, origURL, cloneURL, regenURL;
    if (log_texture_view(context, inputTexture, &inputURL) &&
        log_pixels(pixelsFP, renderSize, &origURL) &&
        log_pixels(pixelsClone, renderSize, &cloneURL) &&
        log_pixels(pixelsRegen, renderSize, &regenURL)) {
        ERRORF(reporter,
               "\nInput image:\n%s\n\n"
               "==========================================================="
               "\n\n"
               "Orig output image:\n%s\n"
               "==========================================================="
               "\n\n"
               "Clone output image:\n%s\n"
               "==========================================================="
               "\n\n"
               "Regen output image:\n%s\n",
               inputURL.c_str(), origURL.c_str(), cloneURL.c_str(), regenURL.c_str());
    }
}

// Tests that a fragment processor returned by GrFragmentProcessor::clone() is equivalent to its
// progenitor.
DEF_GPUTEST_FOR_GL_RENDERING_CONTEXTS(ProcessorCloneTest, reporter, ctxInfo) {
    GrDirectContext* context = ctxInfo.directContext();
    GrResourceProvider* resourceProvider = context->priv().resourceProvider();

    TestFPGenerator fpGenerator{context, resourceProvider};
    if (!fpGenerator.init()) {
        ERRORF(reporter, "Could not initialize TestFPGenerator");
        return;
    }

    // Make the destination context for the test.
    static constexpr int kRenderSize = 1024;
    auto sdc = skgpu::v1::SurfaceDrawContext::Make(
            context, GrColorType::kRGBA_8888, nullptr, SkBackingFit::kExact,
            {kRenderSize, kRenderSize}, SkSurfaceProps());

    std::vector<GrColor> inputPixels = make_input_pixels(kRenderSize, kRenderSize, 0.0f);
    GrSurfaceProxyView inputTexture =
            make_input_texture(context, kRenderSize, kRenderSize, inputPixels.data());

    // On failure we write out images, but just write the first failing set as the print is very
    // large.
    bool loggedFirstFailure = false;

    // Storage for the original frame's readback and the readback of its clone.
    std::vector<GrColor> readDataFP(kRenderSize * kRenderSize);
    std::vector<GrColor> readDataClone(kRenderSize * kRenderSize);
    std::vector<GrColor> readDataRegen(kRenderSize * kRenderSize);

    // Because processor factories configure themselves in random ways, this is not exhaustive.
    for (int i = 0; i < GrFragmentProcessorTestFactory::Count(); ++i) {
        static constexpr int kTimesToInvokeFactory = 10;
        for (int j = 0; j < kTimesToInvokeFactory; ++j) {
            fpGenerator.reroll();
            std::unique_ptr<GrFragmentProcessor> fp =
                    fpGenerator.make(i, /*randomTreeDepth=*/1, /*inputFP=*/nullptr);
            std::unique_ptr<GrFragmentProcessor> regen =
                    fpGenerator.make(i, /*randomTreeDepth=*/1, /*inputFP=*/nullptr);
            std::unique_ptr<GrFragmentProcessor> clone = fp->clone();
            if (!clone) {
                ERRORF(reporter, "Clone of processor %s failed.", fp->dumpTreeInfo().c_str());
                continue;
            }
            assert_processor_equality(reporter, *fp, *clone);

            // Draw with original and read back the results.
            render_fp(context, sdc.get(), std::move(fp), readDataFP.data());

            // Draw with clone and read back the results.
            render_fp(context, sdc.get(), std::move(clone), readDataClone.data());

            // Check that the results are the same.
            if (!verify_identical_render(reporter, kRenderSize, "Processor clone",
                                         readDataFP.data(), readDataClone.data())) {
                // Dump a description from the regenerated processor (since the original FP has
                // already been consumed).
                ERRORF(reporter, "FP hierarchy:\n%s", regen->dumpTreeInfo().c_str());

                // Render and readback output from the regenerated FP. If this also mismatches, the
                // FP itself doesn't generate consistent output. This could happen if:
                // - the FP's TestCreate() does not always generate the same FP from a given seed
                // - the FP's Make() does not always generate the same FP when given the same inputs
                // - the FP itself generates inconsistent pixels (shader UB?)
                // - the driver has a bug
                render_fp(context, sdc.get(), std::move(regen), readDataRegen.data());

                if (!verify_identical_render(reporter, kRenderSize, "Regenerated processor",
                                             readDataFP.data(), readDataRegen.data())) {
                    ERRORF(reporter, "Output from regen did not match original!\n");
                } else {
                    ERRORF(reporter, "Regenerated processor output matches original results.\n");
                }

                // If this is the first time we've encountered a cloning failure, log the generated
                // images to the reporter as data URLs.
                if (!loggedFirstFailure) {
                    log_clone_failure(reporter, kRenderSize, context, inputTexture,
                                      readDataFP.data(), readDataClone.data(),
                                      readDataRegen.data());
                    loggedFirstFailure = true;
                }
            }
        }
    }
}

#endif  // GR_TEST_UTILS
