/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"
#include "Test.h"

#include "GrClip.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrGpuResource.h"
#include "GrMemoryPool.h"
#include "GrProxyProvider.h"
#include "GrRenderTargetContext.h"
#include "GrRenderTargetContextPriv.h"
#include "GrResourceProvider.h"
#include "effects/GrSkSLFP.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramBuilder.h"
#include "ops/GrMeshDrawOp.h"
#include "ops/GrRectOpFactory.h"
#include "TestUtils.h"

#include <random>

namespace {
class TestOp : public GrMeshDrawOp {
public:
    DEFINE_OP_CLASS_ID
    static std::unique_ptr<GrDrawOp> Make(GrContext* context,
                                          std::unique_ptr<GrFragmentProcessor> fp) {
        GrOpMemoryPool* pool = context->contextPriv().opMemoryPool();

        return pool->allocate<TestOp>(std::move(fp));
    }

    const char* name() const override { return "TestOp"; }

    void visitProxies(const VisitProxyFunc& func) const override {
        fProcessors.visitProxies(func);
    }

    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }

    RequiresDstTexture finalize(const GrCaps& caps, const GrAppliedClip* clip) override {
        static constexpr GrProcessorAnalysisColor kUnknownColor;
        GrColor overrideColor;
        fProcessors.finalize(kUnknownColor, GrProcessorAnalysisCoverage::kNone, clip, false, caps,
                             &overrideColor);
        return RequiresDstTexture::kNo;
    }

private:
    friend class ::GrOpMemoryPool; // for ctor

    TestOp(std::unique_ptr<GrFragmentProcessor> fp)
            : INHERITED(ClassID()), fProcessors(std::move(fp)) {
        this->setBounds(SkRect::MakeWH(100, 100), HasAABloat::kNo, IsZeroArea::kNo);
    }

    void onPrepareDraws(Target* target) override { return; }

    GrProcessorSet fProcessors;

    typedef GrMeshDrawOp INHERITED;
};

/**
 * FP used to test ref/IO counts on owned GrGpuResources. Can also be a parent FP to test counts
 * of resources owned by child FPs.
 */
class TestFP : public GrFragmentProcessor {
public:
    static std::unique_ptr<GrFragmentProcessor> Make(std::unique_ptr<GrFragmentProcessor> child) {
        return std::unique_ptr<GrFragmentProcessor>(new TestFP(std::move(child)));
    }
    static std::unique_ptr<GrFragmentProcessor> Make(const SkTArray<sk_sp<GrTextureProxy>>& proxies,
                                                     const SkTArray<sk_sp<GrBuffer>>& buffers) {
        return std::unique_ptr<GrFragmentProcessor>(new TestFP(proxies, buffers));
    }

    const char* name() const override { return "test"; }

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const override {
        // We don't really care about reusing these.
        static int32_t gKey = 0;
        b->add32(sk_atomic_inc(&gKey));
    }

    std::unique_ptr<GrFragmentProcessor> clone() const override {
        return std::unique_ptr<GrFragmentProcessor>(new TestFP(*this));
    }

private:
    TestFP(const SkTArray<sk_sp<GrTextureProxy>>& proxies, const SkTArray<sk_sp<GrBuffer>>& buffers)
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

// FP that sets sk_InColor = premul(unpremul(inputData) + inputOffset) for its child processor.
// However, the shader is given a fixed, positive offset. If the inputData's color is too close to
// 1.0, adding the offset will cause it to go outside the [0, 1] range. To accommodate this, the
// shader checks each input channel and either adds or subtracts the offset to keep it in range.
// This assumes the offset will be less than 0.5, which ensures that the modulating color will not
// cause the FP to saturate.
class OptimizationTestFP : public GrFragmentProcessor {
public:
    static std::unique_ptr<GrFragmentProcessor> Make(
            std::unique_ptr<GrFragmentProcessor> processor,
            sk_sp<GrTextureProxy> inputData,
            SkScalar inputOffset) {
        return std::unique_ptr<GrFragmentProcessor>(
                new OptimizationTestFP(std::move(processor), std::move(inputData), inputOffset));
    }

    const char* name() const override { return "OptimizationTest"; }

    std::unique_ptr<GrFragmentProcessor> clone() const override {
        return std::unique_ptr<GrFragmentProcessor>(new OptimizationTestFP(*this));
    }

private:
    OptimizationTestFP(std::unique_ptr<GrFragmentProcessor> processor,
                       sk_sp<GrTextureProxy> inputData, SkScalar inputOffset)
            : INHERITED(kOptimizationTestFP_ClassID, kNone_OptimizationFlags)
            , fInputData(std::move(inputData), GrSamplerState(GrSamplerState::WrapMode::kClamp,
                                                              GrSamplerState::Filter::kNearest))
            , fImageCoordTransform(SkMatrix::I(), fInputData.proxy())
            , fInputOffset(inputOffset) {
        this->registerChildProcessor(std::move(processor));
        this->setTextureSamplerCnt(1);
        this->addCoordTransform(&fImageCoordTransform);
    }
    OptimizationTestFP(const OptimizationTestFP& toClone)
            : INHERITED(kOptimizationTestFP_ClassID, kNone_OptimizationFlags)
            , fInputData(toClone.fInputData)
            , fImageCoordTransform(toClone.fImageCoordTransform)
            , fInputOffset(toClone.fInputOffset) {
        this->registerChildProcessor(toClone.childProcessor(0).clone());
        this->setTextureSamplerCnt(1);
        this->addCoordTransform(&fImageCoordTransform);
    }

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override {
        class GLFP : public GrGLSLFragmentProcessor {
        public:
            void emitCode(EmitArgs& args) override {
                GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
                const OptimizationTestFP& _outer = args.fFp.cast<OptimizationTestFP>();
                (void)_outer;

                fInputOffsetVar = args.fUniformHandler->addUniform(
                        kFragment_GrShaderFlag, kFloat_GrSLType, kDefault_GrSLPrecision, "offset");

                // Get premul input data
                SkString sk_TransformedCoords2D_0 = fragBuilder->ensureCoords2D(
                        args.fTransformedCoords[0]);
                fragBuilder->codeAppendf(
                        "half4 inputData = texture(%s, %s).%s;\n",
                        fragBuilder->getProgramBuilder()->samplerVariable(args.fTexSamplers[0]).c_str(),
                        sk_TransformedCoords2D_0.c_str(),
                        fragBuilder->getProgramBuilder()->samplerSwizzle(args.fTexSamplers[0]).c_str());
                // Convert it into unpremul(input) and apply the offset per channel, then make
                // it premul again.
                fragBuilder->codeAppendf(
                        "inputData = half4(inputData.rgb / inputData.a, inputData.a);\n"
                        "half offset = %s;\n"
                        "inputData.r += inputData.r > 0.5 ? -offset : offset;\n"
                        "inputData.g += inputData.g > 0.5 ? -offset : offset;\n"
                        "inputData.b += inputData.b > 0.5 ? -offset : offset;\n"
                        "inputData.a += inputData.a > 0.5 ? -offset : offset;\n"
                        "inputData.rgb *= inputData.a;\n",
                        args.fUniformHandler->getUniformCStr(fInputOffsetVar));
                // Now output child shader with inputData as its input color
                this->emitChild(0, "inputData", args);
            }
        private:
            void onSetData(const GrGLSLProgramDataManager& pdman,
                           const GrFragmentProcessor& _proc) override {
                const OptimizationTestFP& _outer = _proc.cast<OptimizationTestFP>();
                pdman.set1f(fInputOffsetVar, _outer.fInputOffset);
            }

            UniformHandle fInputOffsetVar;
        };
        return new GLFP;
    }

    const TextureSampler& onTextureSampler(int index) const override {
        return IthTextureSampler(index, fInputData);
    }

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {}

    bool onIsEqual(const GrFragmentProcessor& other) const override {
            const OptimizationTestFP& that = other.cast<OptimizationTestFP>();
        (void)that;
        if (fInputData != that.fInputData) return false;
        if (fInputOffset != that.fInputOffset) return false;
        return true;
    }

    typedef GrFragmentProcessor INHERITED;

    TextureSampler fInputData;
    GrCoordTransform fImageCoordTransform;
    SkScalar fInputOffset;
};

}

template <typename T>
inline void testingOnly_getIORefCnts(const T* resource, int* refCnt, int* readCnt, int* writeCnt) {
    *refCnt = resource->fRefCnt;
    *readCnt = resource->fPendingReads;
    *writeCnt = resource->fPendingWrites;
}

void testingOnly_getIORefCnts(GrTextureProxy* proxy, int* refCnt, int* readCnt, int* writeCnt) {
    *refCnt = proxy->getBackingRefCnt_TestOnly();
    *readCnt = proxy->getPendingReadCnt_TestOnly();
    *writeCnt = proxy->getPendingWriteCnt_TestOnly();
}

DEF_GPUTEST_FOR_ALL_CONTEXTS(ProcessorRefTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    GrProxyProvider* proxyProvider = context->contextPriv().proxyProvider();

    GrSurfaceDesc desc;
    desc.fWidth = 10;
    desc.fHeight = 10;
    desc.fConfig = kRGBA_8888_GrPixelConfig;

    for (bool makeClone : {false, true}) {
        for (int parentCnt = 0; parentCnt < 2; parentCnt++) {
            sk_sp<GrRenderTargetContext> renderTargetContext(
                    context->contextPriv().makeDeferredRenderTargetContext(
                                                             SkBackingFit::kApprox, 1, 1,
                                                             kRGBA_8888_GrPixelConfig, nullptr));
            {
                sk_sp<GrTextureProxy> proxy1 = proxyProvider->createProxy(
                        desc, kTopLeft_GrSurfaceOrigin, SkBackingFit::kExact, SkBudgeted::kYes);
                sk_sp<GrTextureProxy> proxy2 = proxyProvider->createProxy(
                        desc, kTopLeft_GrSurfaceOrigin, SkBackingFit::kExact, SkBudgeted::kYes);
                sk_sp<GrTextureProxy> proxy3 = proxyProvider->createProxy(
                        desc, kTopLeft_GrSurfaceOrigin, SkBackingFit::kExact, SkBudgeted::kYes);
                sk_sp<GrTextureProxy> proxy4 = proxyProvider->createProxy(
                        desc, kTopLeft_GrSurfaceOrigin, SkBackingFit::kExact, SkBudgeted::kYes);
                {
                    SkTArray<sk_sp<GrTextureProxy>> proxies;
                    SkTArray<sk_sp<GrBuffer>> buffers;
                    proxies.push_back(proxy1);
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
                int refCnt, readCnt, writeCnt;

                testingOnly_getIORefCnts(proxy1.get(), &refCnt, &readCnt, &writeCnt);
                // IO counts should be double if there is a clone of the FP.
                int ioRefMul = makeClone ? 2 : 1;
                REPORTER_ASSERT(reporter, -1 == refCnt);
                REPORTER_ASSERT(reporter, ioRefMul * 1 == readCnt);
                REPORTER_ASSERT(reporter, ioRefMul * 0 == writeCnt);

                context->flush();

                testingOnly_getIORefCnts(proxy1.get(), &refCnt, &readCnt, &writeCnt);
                REPORTER_ASSERT(reporter, 1 == refCnt);
                REPORTER_ASSERT(reporter, ioRefMul * 0 == readCnt);
                REPORTER_ASSERT(reporter, ioRefMul * 0 == writeCnt);

            }
        }
    }
}

// This test uses the random GrFragmentProcessor test factory, which relies on static initializers.
#if SK_ALLOW_STATIC_GLOBAL_INITIALIZERS

#include "SkCommandLineFlags.h"
DEFINE_bool(randomProcessorTest, false, "Use non-deterministic seed for random processor tests?");
DEFINE_uint32(processorSeed, 0, "Use specific seed for processor tests. Overridden by " \
                                "--randomProcessorTest.");

#if GR_TEST_UTILS

static GrColor input_texel_color(int i, int j) {
    GrColor color = GrColorPackRGBA((uint8_t)j, (uint8_t)(i + j), (uint8_t)(2 * j - i), (uint8_t)i);
    return GrPremulColor(color);
}

void test_draw_op(GrContext* context,
                  GrRenderTargetContext* rtc,
                  std::unique_ptr<GrFragmentProcessor> fp,
                  sk_sp<GrTextureProxy> inputDataProxy,
                  SkScalar inputOffset) {
    GrPaint paint;

    // The input color sent to the fp being tested is premul(unpremul(texture) + inputOffset),
    // so that it sees ((r+/-d)*(a+/-d), (g+/-d)*(a+/-d), (b+/-d)*(a+/-d), a+/-d), which
    // differentiates alpha modulation by (a+/-d) and color modulation by (c+/-d)*(a+/-d).
    // OptimizationTestFP implements this logic for us.
    paint.addColorFragmentProcessor(OptimizationTestFP::Make(std::move(fp),
                                    std::move(inputDataProxy), inputOffset));
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);

    auto op = GrRectOpFactory::MakeNonAAFill(context, std::move(paint), SkMatrix::I(),
                                             SkRect::MakeWH(rtc->width(), rtc->height()),
                                             GrAAType::kNone);
    rtc->addDrawOp(GrNoClip(), std::move(op));
}

// This assumes that the output buffer will be the same size as inputDataProxy
void render_fp(GrContext* context, GrRenderTargetContext* rtc,
                    GrFragmentProcessor* fp, sk_sp<GrTextureProxy> inputDataProxy,
                    SkScalar inputOffset, GrColor* buffer) {
    int width = inputDataProxy->width();
    int height = inputDataProxy->height();

    // test_draw_op needs to take ownership of an FP, so give it a clone that it can own
    test_draw_op(context, rtc, fp->clone(), inputDataProxy, inputOffset);
    memset(buffer, 0x0, sizeof(GrColor) * width * height);
           rtc->readPixels(SkImageInfo::Make(width, height, kRGBA_8888_SkColorType,
                                             kPremul_SkAlphaType),
                           buffer, 0, 0, 0);
}

/** Initializes the two test texture proxies that are available to the FP test factories. */
bool init_test_textures(GrProxyProvider* proxyProvider, SkRandom* random,
                        sk_sp<GrTextureProxy> proxies[2]) {
    static const int kTestTextureSize = 256;

    {
        // Put premul data into the RGBA texture that the test FPs can optionally use.
        std::unique_ptr<GrColor[]> rgbaData(new GrColor[kTestTextureSize * kTestTextureSize]);
        for (int y = 0; y < kTestTextureSize; ++y) {
            for (int x = 0; x < kTestTextureSize; ++x) {
                rgbaData[kTestTextureSize * y + x] =
                        input_texel_color(random->nextULessThan(256), random->nextULessThan(256));
            }
        }

        SkImageInfo ii = SkImageInfo::Make(kTestTextureSize, kTestTextureSize,
                                           kRGBA_8888_SkColorType, kPremul_SkAlphaType);
        SkPixmap pixmap(ii, rgbaData.get(), ii.minRowBytes());
        sk_sp<SkImage> img = SkImage::MakeRasterCopy(pixmap);
        proxies[0] = proxyProvider->createTextureProxy(img, kNone_GrSurfaceFlags, 1,
                                                       SkBudgeted::kYes, SkBackingFit::kExact);
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
    }

    return proxies[0] && proxies[1];
}

// Creates a texture of premul colors used as the output of the fragment processor that precedes
// the fragment processor under test. Color values are those provided by input_texel_color().
sk_sp<GrTextureProxy> make_input_texture(GrProxyProvider* proxyProvider, int width, int height) {
    std::unique_ptr<GrColor[]> data(new GrColor[width * height]);
    for (int y = 0; y < width; ++y) {
        for (int x = 0; x < height; ++x) {
            data.get()[width * y + x] = input_texel_color(x, y);
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
    sk_sp<GrSurfaceContext> sContext(context->contextPriv().makeWrappedSurfaceContext(src));
    return log_surface_context(sContext, dst);
}

// Calculate the final input colors used to modulate the FP for delta offset of 1 step (-> i2) and
// 2 steps (-> i3).  The modulating color for 0 steps (-> i1) is just equal to input. This must
// match the behavior of the OptimizationTestFP.
void modulating_colors(const SkPMColor4f& input, SkScalar inputDelta,
                       SkPMColor4f* i2, SkPMColor4f* i3) {
    SkRGBA4f<kUnpremul_SkAlphaType> unPMI1 = input.unpremul();
    SkRGBA4f<kUnpremul_SkAlphaType> unPMI2 = unPMI1;
    SkRGBA4f<kUnpremul_SkAlphaType> unPMI3 = unPMI1;

    for (int i = 0; i < 4; i++) {
        if (unPMI1[i] > 0.5) {
            unPMI2[i] -= inputDelta;
            unPMI3[i] -= 2 * inputDelta;
        } else {
            unPMI2[i] += inputDelta;
            unPMI3[i] += 2 * inputDelta;
        }
    }

    *i2 = unPMI2.premul();
    *i3 = unPMI3.premul();
}

bool fuzzy_color_equals(const GrColor4f c1, const GrColor4f c2) {
    // With the loss of precision of rendering into 32-bit color, then estimating the FP's output
    // from that, it is not uncommon for a valid output to differ from estimate by up to 0.01
    // (really 1/128 ~ .0078, but frequently floating point issues make that tolerance a little
    // too unforgiving).
    static constexpr SkScalar kTolerance = 0.01f;
    for (int i = 0; i < 4; i++) {
        if (!SkScalarNearlyEqual(c1.fRGBA[i], c2.fRGBA[i], kTolerance)) {
            return false;
        }
    }
    return true;
}

// static constexpr SkScalar kTolerance = 1.0f / 128.0f;
// Testing legal alpha modulation:
// - when a constant is added to the all four input channels, and the shader outputs an RGBA
//   modulated by the input alpha, Co = Cfp * Afp * (Ai+/-d) and Ao = Afp * (Ai+/-d).
// - the expected difference between o2 and o1, or o3 and o2 are Co/Ai*d for RGB and Ao/Ai*d
//   for alpha. o3 to o1 is twice that.
// - note that while Cfp is the unpremul RGB, Co is premul
// - the sign of d is negative if Ai > 0.5.
bool legal_alpha_modulation(const GrColor4f& o1, const GrColor4f& o2, const GrColor4f& o3,
                            const SkPMColor4f& input, SkScalar inputDelta) {
    SkPMColor4f input2, input3;
    modulating_colors(input, inputDelta, &input2, &input3);

    // Reconstruct the output of the FP before the shader modulated its color with the input alpha.
    // When the original input alpha is very small, it may cause the final output color to round
    // to 0, in which case we estimate the pre-modulated color using one of the stepped frames that
    // will then have a guaranteed larger alpha (since the offset will be added to it).
    SkPMColor4f fpPreModulation;
    for (int i = 0; i < 4; i++) {
        if (input.fA < 0.2f) {
            // Use the stepped frame
            fpPreModulation[i] = o2.fRGBA[i] / input2.fA;
        } else {
            fpPreModulation[i] = o1.fRGBA[i] / input.fA;
        }
    }

    // With reconstructed pre-modulated FP output, confirm that fp*input.fA == o1, fp*i2.fA == o2
    // and fp*i3.fA == o3.
    GrColor4f expected1 = GrColor4f(fpPreModulation.fR * input.fA, fpPreModulation.fG * input.fA,
                                    fpPreModulation.fB * input.fA, fpPreModulation.fA * input.fA);
    GrColor4f expected2 = GrColor4f(fpPreModulation.fR * input2.fA, fpPreModulation.fG * input2.fA,
                                    fpPreModulation.fB * input2.fA, fpPreModulation.fA * input2.fA);
    GrColor4f expected3 = GrColor4f(fpPreModulation.fR * input3.fA, fpPreModulation.fG * input3.fA,
                                    fpPreModulation.fB * input3.fA, fpPreModulation.fA * input3.fA);

    return fuzzy_color_equals(o1, expected1) &&
           fuzzy_color_equals(o2, expected2) &&
           fuzzy_color_equals(o3, expected3);
}

// Testing legal color modulation:
// - when a constant is added to all four input channels, and the shader outputs an RGBA
//   modulated by the input color, Co = Cfp*Afp * (Ci+d)*(Ai+d) and Ao = Afp * (Ai+d).
// - a valid output alpha channel follows the same check used in legal_alpha_modulation
// - a valid output color value's expected change becomes a quadratic because the input delta is
//   introduced from both Ai and Ci.
//     o2 - o1 -> Co/Ai * d + Co/Ci * d + Co / (Ci*Ai) * d^2
//     o3 - o2 -> Co/Ai * d + Co/Ci * d + 3*Co / (Ci*Ai) * d^2
//     o3 - o1 -> 2*Co/Ai * d + 2*Co/Ci * d + 4*Co / (Ci*Ai) * d^2
// - note that Ci is unpremul given the definition of Co above
bool legal_color_modulation(const GrColor4f& o1, const GrColor4f& o2, const GrColor4f& o3,
                            const SkPMColor4f& input, SkScalar inputDelta) {
    SkPMColor4f input2, input3;
    modulating_colors(input, inputDelta, &input2, &input3);

    SkPMColor4f fpPreModulation;
    for (int i = 0; i < 4; i++) {
        if (input[i] < 0.2f) {
            // Use the stepped frame
            fpPreModulation[i] = o2.fRGBA[i] / input2[i];
        } else {
            fpPreModulation[i] = o1.fRGBA[i] / input[i];
        }
    }

    // With reconstructed pre-modulated FP output, confirm that fp[i]*input[i] == o1,
    // fp[i]*input2[i] == o2 and fp[i]*input3[i] == o3
    GrColor4f expected1 = GrColor4f(fpPreModulation.fR * input.fR, fpPreModulation.fG * input.fG,
                                    fpPreModulation.fB * input.fB, fpPreModulation.fA * input.fA);
    GrColor4f expected2 = GrColor4f(fpPreModulation.fR * input2.fR, fpPreModulation.fG * input2.fG,
                                    fpPreModulation.fB * input2.fB, fpPreModulation.fA * input2.fA);
    GrColor4f expected3 = GrColor4f(fpPreModulation.fR * input3.fR, fpPreModulation.fG * input3.fG,
                                    fpPreModulation.fB * input3.fB, fpPreModulation.fA * input3.fA);

    return fuzzy_color_equals(o1, expected1) &&
           fuzzy_color_equals(o2, expected2) &&
           fuzzy_color_equals(o3, expected3);
}

DEF_GPUTEST_FOR_GL_RENDERING_CONTEXTS(ProcessorOptimizationValidationTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    GrProxyProvider* proxyProvider = context->contextPriv().proxyProvider();
    auto resourceProvider = context->contextPriv().resourceProvider();
    using FPFactory = GrFragmentProcessorTestFactory;

    uint32_t seed = FLAGS_processorSeed;
    SkASSERT(seed == 0);
    if (FLAGS_randomProcessorTest) {
        std::random_device rd;
        seed = rd();
    }
    // If a non-deterministic bot fails this test, check the output to see what seed it used, then
    // use --processorSeed <seed> (without --randomProcessorTest) to reproduce.
    SkRandom random(seed);

    // Make the destination context for the test.
    static constexpr int kRenderSize = 256;
    sk_sp<GrRenderTargetContext> rtc = context->contextPriv().makeDeferredRenderTargetContext(
            SkBackingFit::kExact, kRenderSize, kRenderSize, kRGBA_8888_GrPixelConfig, nullptr);

    sk_sp<GrTextureProxy> proxies[2];
    if (!init_test_textures(proxyProvider, &random, proxies)) {
        ERRORF(reporter, "Could not create test textures");
        return;
    }
    GrProcessorTestData testData(&random, context, rtc.get(), proxies);

    auto inputTexture = make_input_texture(proxyProvider, kRenderSize, kRenderSize);

    // Encoded images are very verbose and this tests many potential images, so only export the
    // first failure (subsequent failures have a reasonable chance of being related).
    bool loggedFirstFailure = false;
    bool loggedFirstWarning = false;

    // Render 3 frames to estimate modulation behavior for coverage compatibility optimization
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
        for (int j = 0; j < timesToInvokeFactory; ++j) {
            fp = FPFactory::MakeIdx(i, &testData);
            if (!fp->instantiate(resourceProvider)) {
                continue;
            }

            if (!fp->hasConstantOutputForConstantInput() && !fp->preservesOpaqueInput() &&
                !fp->compatibleWithCoverageAsAlpha()) {
                continue;
            }

            // Draw three frames, first frame has no offset, second frames add .2 and .4, which
            // should then be present as a fixed difference between the frame outputs if the FP
            // is properly following the modulation requirements of the coverage optimization.
            static constexpr SkScalar kInputDelta = 0.2f;

            if (fp->compatibleWithCoverageAsAlpha()) {
                // 2nd and 3rd frames are only used when checking coverage optimization
                render_fp(context, rtc.get(), fp.get(), inputTexture, kInputDelta, readData2.get());
                render_fp(context, rtc.get(), fp.get(), inputTexture, 2 * kInputDelta, readData3.get());
            }
            // Draw base frame last so that rtc holds the original FP behavior if we need to
            // dump the image to the log.
            render_fp(context, rtc.get(), fp.get(), inputTexture, 0.0f, readData1.get());

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
#if defined(SK_SKQP_GLOBAL_ERROR_TOLERANCE)
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
                    GrColor input = input_texel_color(x, y);
                    GrColor output = readData1.get()[y * kRenderSize + x];

                    SkPMColor4f input4f =
                            GrColor4f::FromGrColor(input).asRGBA4f<kPremul_SkAlphaType>();
                    GrColor4f output4f = GrColor4f::FromGrColor(output);

                    if (fp->compatibleWithCoverageAsAlpha()) {
                        GrColor4f o2 = GrColor4f::FromGrColor(readData2.get()[y * kRenderSize + x]);
                        GrColor4f o3 = GrColor4f::FromGrColor(readData3.get()[y * kRenderSize + x]);

                        // A compatible processor is allowed to modulate either the input color or
                        // just the input alpha.
                        bool legalAlphaModulation = legal_alpha_modulation(
                                output4f, o2, o3, input4f, kInputDelta);
                        bool legalColorModulation = legal_color_modulation(
                                output4f, o2, o3, input4f, kInputDelta);

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

                    SkPMColor4f expected4f;
                    if (fp->hasConstantOutputForConstantInput(input4f, &expected4f)) {
                        float rDiff = fabsf(output4f.fRGBA[0] - expected4f.fR);
                        float gDiff = fabsf(output4f.fRGBA[1] - expected4f.fG);
                        float bDiff = fabsf(output4f.fRGBA[2] - expected4f.fB);
                        float aDiff = fabsf(output4f.fRGBA[3] - expected4f.fA);
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
                                        output4f.fRGBA[0], output4f.fRGBA[1], output4f.fRGBA[2],
                                        output4f.fRGBA[3], expected4f.fR, expected4f.fG,
                                        expected4f.fB, expected4f.fA);
                            }
                        }
                    }
                    if (GrColorIsOpaque(input) && fp->preservesOpaqueInput() &&
                        !GrColorIsOpaque(output)) {
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
                    log_surface_proxy(context, inputTexture, &input);
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
                    log_surface_proxy(context, inputTexture, &input);
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
    GrProxyProvider* proxyProvider = context->contextPriv().proxyProvider();
    auto resourceProvider = context->contextPriv().resourceProvider();

    SkRandom random;

    // Make the destination context for the test.
    static constexpr int kRenderSize = 1024;
    sk_sp<GrRenderTargetContext> rtc = context->contextPriv().makeDeferredRenderTargetContext(
            SkBackingFit::kExact, kRenderSize, kRenderSize, kRGBA_8888_GrPixelConfig, nullptr);

    sk_sp<GrTextureProxy> proxies[2];
    if (!init_test_textures(proxyProvider, &random, proxies)) {
        ERRORF(reporter, "Could not create test textures");
        return;
    }
    GrProcessorTestData testData(&random, context, rtc.get(), proxies);

    auto inputTexture = make_input_texture(proxyProvider, kRenderSize, kRenderSize);
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
            if (!fp->instantiate(resourceProvider) || !clone->instantiate(resourceProvider)) {
                continue;
            }
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
            render_fp(context, rtc.get(), fp.get(), inputTexture, 0.0f, readData1.get());

            // Draw with clone and read back the results.
            render_fp(context, rtc.get(), clone.get(), inputTexture, 0.0f, readData2.get());

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
                               name, input_texel_color(x, y), readData1[idx], readData2[idx]);
                        passing = false;
                    }
                }
            }
        }
    }
}

#endif  // GR_TEST_UTILS
#endif  // SK_ALLOW_STATIC_GLOBAL_INITIALIZERS
