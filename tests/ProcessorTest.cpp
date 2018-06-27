/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"
#include "Test.h"

#if SK_SUPPORT_GPU
#include <random>
#include "GrClip.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrGpuResource.h"
#include "GrProxyProvider.h"
#include "GrRenderTargetContext.h"
#include "GrRenderTargetContextPriv.h"
#include "GrResourceProvider.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "ops/GrMeshDrawOp.h"
#include "ops/GrRectOpFactory.h"

namespace {
class TestOp : public GrMeshDrawOp {
public:
    DEFINE_OP_CLASS_ID
    static std::unique_ptr<GrDrawOp> Make(std::unique_ptr<GrFragmentProcessor> fp) {
        return std::unique_ptr<GrDrawOp>(new TestOp(std::move(fp)));
    }

    const char* name() const override { return "TestOp"; }

    void visitProxies(const VisitProxyFunc& func) const override {
        fProcessors.visitProxies(func);
    }

    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }

    RequiresDstTexture finalize(const GrCaps& caps, const GrAppliedClip* clip,
                                GrPixelConfigIsClamped dstIsClamped) override {
        static constexpr GrProcessorAnalysisColor kUnknownColor;
        GrColor overrideColor;
        fProcessors.finalize(kUnknownColor, GrProcessorAnalysisCoverage::kNone, clip, false, caps,
                             dstIsClamped, &overrideColor);
        return RequiresDstTexture::kNo;
    }

private:
    TestOp(std::unique_ptr<GrFragmentProcessor> fp)
            : INHERITED(ClassID()), fProcessors(std::move(fp)) {
        this->setBounds(SkRect::MakeWH(100, 100), HasAABloat::kNo, IsZeroArea::kNo);
    }

    void onPrepareDraws(Target* target) override { return; }

    bool onCombineIfPossible(GrOp* op, const GrCaps& caps) override { return false; }

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
            : INHERITED(kTestFP_ClassID, kNone_OptimizationFlags), fSamplers(4), fBuffers(4) {
        for (const auto& proxy : proxies) {
            this->addTextureSampler(&fSamplers.emplace_back(proxy));
        }
        for (const auto& buffer : buffers) {
            this->addBufferAccess(&fBuffers.emplace_back(kRGBA_8888_GrPixelConfig, buffer.get()));
        }
    }

    TestFP(std::unique_ptr<GrFragmentProcessor> child)
            : INHERITED(kTestFP_ClassID, kNone_OptimizationFlags), fSamplers(4), fBuffers(4) {
        this->registerChildProcessor(std::move(child));
    }

    explicit TestFP(const TestFP& that)
            : INHERITED(kTestFP_ClassID, that.optimizationFlags()), fSamplers(4), fBuffers(4) {
        for (int i = 0; i < that.fSamplers.count(); ++i) {
            fSamplers.emplace_back(that.fSamplers[i]);
            this->addTextureSampler(&fSamplers.back());
        }
        for (int i = 0; i < that.fBuffers.count(); ++i) {
            fBuffers.emplace_back(that.fBuffers[i]);
            this->addBufferAccess(&fBuffers.back());
        }
        for (int i = 0; i < that.numChildProcessors(); ++i) {
            this->registerChildProcessor(that.childProcessor(i).clone());
        }
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

    GrTAllocator<TextureSampler> fSamplers;
    GrTAllocator<BufferAccess> fBuffers;
    typedef GrFragmentProcessor INHERITED;
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
    GrResourceProvider* resourceProvider = context->contextPriv().resourceProvider();

    GrSurfaceDesc desc;
    desc.fOrigin = kTopLeft_GrSurfaceOrigin;
    desc.fWidth = 10;
    desc.fHeight = 10;
    desc.fConfig = kRGBA_8888_GrPixelConfig;

    for (bool makeClone : {false, true}) {
        for (int parentCnt = 0; parentCnt < 2; parentCnt++) {
            sk_sp<GrRenderTargetContext> renderTargetContext(
                    context->makeDeferredRenderTargetContext( SkBackingFit::kApprox, 1, 1,
                                                              kRGBA_8888_GrPixelConfig, nullptr));
            {
                bool texelBufferSupport = context->caps()->shaderCaps()->texelBufferSupport();
                sk_sp<GrTextureProxy> proxy1 =
                        proxyProvider->createProxy(desc, SkBackingFit::kExact, SkBudgeted::kYes);
                sk_sp<GrTextureProxy> proxy2 =
                        proxyProvider->createProxy(desc, SkBackingFit::kExact, SkBudgeted::kYes);
                sk_sp<GrTextureProxy> proxy3 =
                        proxyProvider->createProxy(desc, SkBackingFit::kExact, SkBudgeted::kYes);
                sk_sp<GrTextureProxy> proxy4 =
                        proxyProvider->createProxy(desc, SkBackingFit::kExact, SkBudgeted::kYes);
                sk_sp<GrBuffer> buffer(texelBufferSupport
                        ? resourceProvider->createBuffer(
                                  1024, GrBufferType::kTexel_GrBufferType,
                                  GrAccessPattern::kStatic_GrAccessPattern, 0)
                        : nullptr);
                {
                    SkTArray<sk_sp<GrTextureProxy>> proxies;
                    SkTArray<sk_sp<GrBuffer>> buffers;
                    proxies.push_back(proxy1);
                    if (texelBufferSupport) {
                        buffers.push_back(buffer);
                    }
                    auto fp = TestFP::Make(std::move(proxies), std::move(buffers));
                    for (int i = 0; i < parentCnt; ++i) {
                        fp = TestFP::Make(std::move(fp));
                    }
                    std::unique_ptr<GrFragmentProcessor> clone;
                    if (makeClone) {
                        clone = fp->clone();
                    }
                    std::unique_ptr<GrDrawOp> op(TestOp::Make(std::move(fp)));
                    renderTargetContext->priv().testingOnly_addDrawOp(std::move(op));
                    if (clone) {
                        op = TestOp::Make(std::move(clone));
                        renderTargetContext->priv().testingOnly_addDrawOp(std::move(op));
                    }
                }
                int refCnt, readCnt, writeCnt;

                testingOnly_getIORefCnts(proxy1.get(), &refCnt, &readCnt, &writeCnt);
                // IO counts should be double if there is a clone of the FP.
                int ioRefMul = makeClone ? 2 : 1;
                REPORTER_ASSERT(reporter, 1 == refCnt);
                REPORTER_ASSERT(reporter, ioRefMul * 1 == readCnt);
                REPORTER_ASSERT(reporter, ioRefMul * 0 == writeCnt);

                if (texelBufferSupport) {
                    testingOnly_getIORefCnts(buffer.get(), &refCnt, &readCnt, &writeCnt);
                    REPORTER_ASSERT(reporter, 1 == refCnt);
                    REPORTER_ASSERT(reporter, ioRefMul * 1 == readCnt);
                    REPORTER_ASSERT(reporter, ioRefMul *  0 == writeCnt);
                }

                context->flush();

                testingOnly_getIORefCnts(proxy1.get(), &refCnt, &readCnt, &writeCnt);
                REPORTER_ASSERT(reporter, 1 == refCnt);
                REPORTER_ASSERT(reporter, ioRefMul * 0 == readCnt);
                REPORTER_ASSERT(reporter, ioRefMul * 0 == writeCnt);

                if (texelBufferSupport) {
                    testingOnly_getIORefCnts(buffer.get(), &refCnt, &readCnt, &writeCnt);
                    REPORTER_ASSERT(reporter, 1 == refCnt);
                    REPORTER_ASSERT(reporter, ioRefMul * 0 == readCnt);
                    REPORTER_ASSERT(reporter, ioRefMul * 0 == writeCnt);
                }

                if (texelBufferSupport) {
                    testingOnly_getIORefCnts(proxy2.get(), &refCnt, &readCnt, &writeCnt);
                    REPORTER_ASSERT(reporter, 1 == refCnt);
                    REPORTER_ASSERT(reporter, ioRefMul * 0 == readCnt);
                    REPORTER_ASSERT(reporter, ioRefMul * 0 == writeCnt);

                    testingOnly_getIORefCnts(proxy3.get(), &refCnt, &readCnt, &writeCnt);
                    REPORTER_ASSERT(reporter, 1 == refCnt);
                    REPORTER_ASSERT(reporter, ioRefMul * 0 == readCnt);
                    REPORTER_ASSERT(reporter, ioRefMul * 0 == writeCnt);

                    testingOnly_getIORefCnts(proxy4.get(), &refCnt, &readCnt, &writeCnt);
                    REPORTER_ASSERT(reporter, 1 == refCnt);
                    REPORTER_ASSERT(reporter, ioRefMul * 0 == readCnt);
                    REPORTER_ASSERT(reporter, ioRefMul * 0 == writeCnt);
                }
            }
        }
    }
}

// This test uses the random GrFragmentProcessor test factory, which relies on static initializers.
#if SK_ALLOW_STATIC_GLOBAL_INITIALIZERS

#include "SkCommandLineFlags.h"
DEFINE_bool(randomProcessorTest, false, "Use non-deterministic seed for random processor tests?");

#if GR_TEST_UTILS

static GrColor input_texel_color(int i, int j) {
    GrColor color = GrColorPackRGBA((uint8_t)j, (uint8_t)(i + j), (uint8_t)(2 * j - i), (uint8_t)i);
    return GrPremulColor(color);
}

static GrColor4f input_texel_color4f(int i, int j) {
    return GrColor4f::FromGrColor(input_texel_color(i, j));
}

void test_draw_op(GrRenderTargetContext* rtc, std::unique_ptr<GrFragmentProcessor> fp,
                  sk_sp<GrTextureProxy> inputDataProxy) {
    GrPaint paint;
    paint.addColorTextureProcessor(std::move(inputDataProxy), SkMatrix::I());
    paint.addColorFragmentProcessor(std::move(fp));
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);

    auto op = GrRectOpFactory::MakeNonAAFill(std::move(paint), SkMatrix::I(),
                                             SkRect::MakeWH(rtc->width(), rtc->height()),
                                             GrAAType::kNone);
    rtc->addDrawOp(GrNoClip(), std::move(op));
}

/** Initializes the two test texture proxies that are available to the FP test factories. */
bool init_test_textures(GrProxyProvider* proxyProvider, SkRandom* random,
                        sk_sp<GrTextureProxy> proxies[2]) {
    static const int kTestTextureSize = 256;
    GrSurfaceDesc desc;
    desc.fOrigin = kBottomLeft_GrSurfaceOrigin;
    desc.fWidth = kTestTextureSize;
    desc.fHeight = kTestTextureSize;
    desc.fConfig = kRGBA_8888_GrPixelConfig;

    {
        // Put premul data into the RGBA texture that the test FPs can optionally use.
        std::unique_ptr<GrColor[]> rgbaData(new GrColor[kTestTextureSize * kTestTextureSize]);
        for (int y = 0; y < kTestTextureSize; ++y) {
            for (int x = 0; x < kTestTextureSize; ++x) {
                rgbaData[kTestTextureSize * y + x] =
                        input_texel_color(random->nextULessThan(256), random->nextULessThan(256));
            }
        }

        proxies[0] = proxyProvider->createTextureProxy(desc, SkBudgeted::kYes,
                                                       rgbaData.get(),
                                                       kTestTextureSize * sizeof(GrColor));
    }

    {
        // Put random values into the alpha texture that the test FPs can optionally use.
        desc.fConfig = kAlpha_8_GrPixelConfig;
        std::unique_ptr<uint8_t[]> alphaData(new uint8_t[kTestTextureSize * kTestTextureSize]);
        for (int y = 0; y < kTestTextureSize; ++y) {
            for (int x = 0; x < kTestTextureSize; ++x) {
                alphaData[kTestTextureSize * y + x] = random->nextULessThan(256);
            }
        }

        proxies[1] = proxyProvider->createTextureProxy(desc, SkBudgeted::kYes,
                                                       alphaData.get(), kTestTextureSize);
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
    GrSurfaceDesc desc;
    desc.fOrigin = kBottomLeft_GrSurfaceOrigin;
    desc.fWidth = width;
    desc.fHeight = height;
    desc.fConfig = kRGBA_8888_GrPixelConfig;

    return proxyProvider->createTextureProxy(desc, SkBudgeted::kYes,
                                             data.get(), width * sizeof(GrColor));
}

DEF_GPUTEST_FOR_GL_RENDERING_CONTEXTS(ProcessorOptimizationValidationTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    GrProxyProvider* proxyProvider = context->contextPriv().proxyProvider();
    auto resourceProvider = context->contextPriv().resourceProvider();
    using FPFactory = GrFragmentProcessorTestFactory;

    uint32_t seed = 0;
    if (FLAGS_randomProcessorTest) {
        std::random_device rd;
        seed = rd();
    }
    // If a non-deterministic bot fails this test, check the output to see what seed it used, then
    // hard-code that value here:
    SkRandom random(seed);

    // Make the destination context for the test.
    static constexpr int kRenderSize = 256;
    sk_sp<GrRenderTargetContext> rtc = context->makeDeferredRenderTargetContext(
            SkBackingFit::kExact, kRenderSize, kRenderSize, kRGBA_8888_GrPixelConfig, nullptr);

    sk_sp<GrTextureProxy> proxies[2];
    if (!init_test_textures(proxyProvider, &random, proxies)) {
        ERRORF(reporter, "Could not create test textures");
        return;
    }
    GrProcessorTestData testData(&random, context, rtc.get(), proxies);

    auto inputTexture = make_input_texture(proxyProvider, kRenderSize, kRenderSize);

    std::unique_ptr<GrColor[]> readData(new GrColor[kRenderSize * kRenderSize]);
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

            // Since we transfer away ownership of the original FP, we make a clone.
            auto clone = fp->clone();

            test_draw_op(rtc.get(), std::move(fp), inputTexture);
            memset(readData.get(), 0x0, sizeof(GrColor) * kRenderSize * kRenderSize);
            rtc->readPixels(SkImageInfo::Make(kRenderSize, kRenderSize, kRGBA_8888_SkColorType,
                                              kPremul_SkAlphaType),
                            readData.get(), 0, 0, 0);
            bool passing = true;
            if (0) {  // Useful to see what FPs are being tested.
                SkString children;
                for (int c = 0; c < clone->numChildProcessors(); ++c) {
                    if (!c) {
                        children.append("(");
                    }
                    children.append(clone->name());
                    children.append(c == clone->numChildProcessors() - 1 ? ")" : ", ");
                }
                SkDebugf("%s %s\n", clone->name(), children.c_str());
            }
            for (int y = 0; y < kRenderSize && passing; ++y) {
                for (int x = 0; x < kRenderSize && passing; ++x) {
                    GrColor input = input_texel_color(x, y);
                    GrColor output = readData.get()[y * kRenderSize + x];
                    if (clone->compatibleWithCoverageAsAlpha()) {
                        // A modulating processor is allowed to modulate either the input color or
                        // just the input alpha.
                        bool legalColorModulation =
                                GrColorUnpackA(output) <= GrColorUnpackA(input) &&
                                GrColorUnpackR(output) <= GrColorUnpackR(input) &&
                                GrColorUnpackG(output) <= GrColorUnpackG(input) &&
                                GrColorUnpackB(output) <= GrColorUnpackB(input);
                        bool legalAlphaModulation =
                                GrColorUnpackA(output) <= GrColorUnpackA(input) &&
                                GrColorUnpackR(output) <= GrColorUnpackA(input) &&
                                GrColorUnpackG(output) <= GrColorUnpackA(input) &&
                                GrColorUnpackB(output) <= GrColorUnpackA(input);
                        if (!legalColorModulation && !legalAlphaModulation) {
                            ERRORF(reporter,
                                   "\"Modulating\" processor %s made color/alpha value larger. "
                                   "Input: 0x%08x, Output: 0x%08x, pixel (%d, %d).",
                                   clone->name(), input, output, x, y);
                            passing = false;
                        }
                    }
                    GrColor4f input4f = input_texel_color4f(x, y);
                    GrColor4f output4f = GrColor4f::FromGrColor(output);
                    GrColor4f expected4f;
                    if (clone->hasConstantOutputForConstantInput(input4f, &expected4f)) {
                        float rDiff = fabsf(output4f.fRGBA[0] - expected4f.fRGBA[0]);
                        float gDiff = fabsf(output4f.fRGBA[1] - expected4f.fRGBA[1]);
                        float bDiff = fabsf(output4f.fRGBA[2] - expected4f.fRGBA[2]);
                        float aDiff = fabsf(output4f.fRGBA[3] - expected4f.fRGBA[3]);
                        static constexpr float kTol = 4 / 255.f;
                        if (rDiff > kTol || gDiff > kTol || bDiff > kTol || aDiff > kTol) {
                            ERRORF(reporter,
                                   "Processor %s claimed output for const input doesn't match "
                                   "actual output. Error: %f, Tolerance: %f, input: (%f, %f, %f, "
                                   "%f), actual: (%f, %f, %f, %f), expected(%f, %f, %f, %f)",
                                   clone->name(),
                                   SkTMax(rDiff, SkTMax(gDiff, SkTMax(bDiff, aDiff))), kTol,
                                   input4f.fRGBA[0], input4f.fRGBA[1], input4f.fRGBA[2],
                                   input4f.fRGBA[3], output4f.fRGBA[0], output4f.fRGBA[1],
                                   output4f.fRGBA[2], output4f.fRGBA[3], expected4f.fRGBA[0],
                                   expected4f.fRGBA[1], expected4f.fRGBA[2], expected4f.fRGBA[3]);
                            passing = false;
                        }
                    }
                    if (GrColorIsOpaque(input) && clone->preservesOpaqueInput() &&
                        !GrColorIsOpaque(output)) {
                        ERRORF(reporter,
                               "Processor %s claimed opaqueness is preserved but it is not. Input: "
                               "0x%08x, Output: 0x%08x.",
                               clone->name(), input, output);
                        passing = false;
                    }
                    if (!passing) {
                        ERRORF(reporter, "Seed: 0x%08x, Processor details: %s", seed,
                               clone->dumpInfo().c_str());
                    }
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
    sk_sp<GrRenderTargetContext> rtc = context->makeDeferredRenderTargetContext(
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
            test_draw_op(rtc.get(), std::move(fp), inputTexture);
            memset(readData1.get(), 0x0, sizeof(GrColor) * kRenderSize * kRenderSize);
            rtc->readPixels(readInfo, readData1.get(), 0, 0, 0);

            // Draw with clone and read back the results.
            test_draw_op(rtc.get(), std::move(clone), inputTexture);
            memset(readData2.get(), 0x0, sizeof(GrColor) * kRenderSize * kRenderSize);
            rtc->readPixels(readInfo, readData2.get(), 0, 0, 0);

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
#endif  // SK_SUPPORT_GPU
