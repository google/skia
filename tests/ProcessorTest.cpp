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
#include "GrGpuResource.h"
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
    const char* name() const override { return "TestOp"; }

    static std::unique_ptr<GrDrawOp> Make(sk_sp<GrFragmentProcessor> fp) {
        return std::unique_ptr<GrDrawOp>(new TestOp(std::move(fp)));
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
    TestOp(sk_sp<GrFragmentProcessor> fp) : INHERITED(ClassID()), fProcessors(std::move(fp)) {
        this->setBounds(SkRect::MakeWH(100, 100), HasAABloat::kNo, IsZeroArea::kNo);
    }

    void onPrepareDraws(Target* target) const override { return; }

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
    struct Image {
        Image(sk_sp<GrTextureProxy> proxy, GrIOType ioType) : fProxy(proxy), fIOType(ioType) {}
        sk_sp<GrTextureProxy> fProxy;
        GrIOType fIOType;
    };
    static sk_sp<GrFragmentProcessor> Make(sk_sp<GrFragmentProcessor> child) {
        return sk_sp<GrFragmentProcessor>(new TestFP(std::move(child)));
    }
    static sk_sp<GrFragmentProcessor> Make(const SkTArray<sk_sp<GrTextureProxy>>& proxies,
                                           const SkTArray<sk_sp<GrBuffer>>& buffers,
                                           const SkTArray<Image>& images) {
        return sk_sp<GrFragmentProcessor>(new TestFP(proxies, buffers, images));
    }

    const char* name() const override { return "test"; }

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const override {
        // We don't really care about reusing these.
        static int32_t gKey = 0;
        b->add32(sk_atomic_inc(&gKey));
    }

private:
    TestFP(const SkTArray<sk_sp<GrTextureProxy>>& proxies,
           const SkTArray<sk_sp<GrBuffer>>& buffers,
           const SkTArray<Image>& images)
            : INHERITED(kNone_OptimizationFlags), fSamplers(4), fBuffers(4), fImages(4) {
        for (const auto& proxy : proxies) {
            this->addTextureSampler(&fSamplers.emplace_back(proxy));
        }
        for (const auto& buffer : buffers) {
            this->addBufferAccess(&fBuffers.emplace_back(kRGBA_8888_GrPixelConfig, buffer.get()));
        }
        for (const Image& image : images) {
            fImages.emplace_back(image.fProxy, image.fIOType,
                                 GrSLMemoryModel::kNone, GrSLRestrict::kNo);
            this->addImageStorageAccess(&fImages.back());
        }
    }

    TestFP(sk_sp<GrFragmentProcessor> child)
            : INHERITED(kNone_OptimizationFlags), fSamplers(4), fBuffers(4), fImages(4) {
        this->registerChildProcessor(std::move(child));
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
    GrTAllocator<ImageStorageAccess> fImages;
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

    GrTextureDesc desc;
    desc.fConfig = kRGBA_8888_GrPixelConfig;
    desc.fWidth = 10;
    desc.fHeight = 10;

    for (int parentCnt = 0; parentCnt < 2; parentCnt++) {
        sk_sp<GrRenderTargetContext> renderTargetContext(context->makeDeferredRenderTargetContext(
                SkBackingFit::kApprox, 1, 1, kRGBA_8888_GrPixelConfig, nullptr));
        {
            bool texelBufferSupport = context->caps()->shaderCaps()->texelBufferSupport();
            bool imageLoadStoreSupport = context->caps()->shaderCaps()->imageLoadStoreSupport();
            sk_sp<GrTextureProxy> proxy1(GrSurfaceProxy::MakeDeferred(context->resourceProvider(),
                                                                      desc, SkBackingFit::kExact,
                                                                      SkBudgeted::kYes));
            sk_sp<GrTextureProxy> proxy2(GrSurfaceProxy::MakeDeferred(context->resourceProvider(),
                                                                      desc, SkBackingFit::kExact,
                                                                      SkBudgeted::kYes));
            sk_sp<GrTextureProxy> proxy3(GrSurfaceProxy::MakeDeferred(context->resourceProvider(),
                                                                      desc, SkBackingFit::kExact,
                                                                      SkBudgeted::kYes));
            sk_sp<GrTextureProxy> proxy4(GrSurfaceProxy::MakeDeferred(context->resourceProvider(),
                                                                      desc, SkBackingFit::kExact,
                                                                      SkBudgeted::kYes));
            sk_sp<GrBuffer> buffer(texelBufferSupport
                                           ? context->resourceProvider()->createBuffer(
                                                     1024, GrBufferType::kTexel_GrBufferType,
                                                     GrAccessPattern::kStatic_GrAccessPattern, 0)
                                           : nullptr);
            {
                SkTArray<sk_sp<GrTextureProxy>> proxies;
                SkTArray<sk_sp<GrBuffer>> buffers;
                SkTArray<TestFP::Image> images;
                proxies.push_back(proxy1);
                if (texelBufferSupport) {
                    buffers.push_back(buffer);
                }
                if (imageLoadStoreSupport) {
                    images.emplace_back(proxy2, GrIOType::kRead_GrIOType);
                    images.emplace_back(proxy3, GrIOType::kWrite_GrIOType);
                    images.emplace_back(proxy4, GrIOType::kRW_GrIOType);
                }
                auto fp = TestFP::Make(std::move(proxies), std::move(buffers), std::move(images));
                for (int i = 0; i < parentCnt; ++i) {
                    fp = TestFP::Make(std::move(fp));
                }
                std::unique_ptr<GrDrawOp> op(TestOp::Make(std::move(fp)));
                renderTargetContext->priv().testingOnly_addDrawOp(std::move(op));
            }
            int refCnt, readCnt, writeCnt;

            testingOnly_getIORefCnts(proxy1.get(), &refCnt, &readCnt, &writeCnt);
            REPORTER_ASSERT(reporter, 1 == refCnt);
            REPORTER_ASSERT(reporter, 1 == readCnt);
            REPORTER_ASSERT(reporter, 0 == writeCnt);

            if (texelBufferSupport) {
                testingOnly_getIORefCnts(buffer.get(), &refCnt, &readCnt, &writeCnt);
                REPORTER_ASSERT(reporter, 1 == refCnt);
                REPORTER_ASSERT(reporter, 1 == readCnt);
                REPORTER_ASSERT(reporter, 0 == writeCnt);
            }

            if (imageLoadStoreSupport) {
                testingOnly_getIORefCnts(proxy2.get(), &refCnt, &readCnt, &writeCnt);
                REPORTER_ASSERT(reporter, 1 == refCnt);
                REPORTER_ASSERT(reporter, 1 == readCnt);
                REPORTER_ASSERT(reporter, 0 == writeCnt);

                testingOnly_getIORefCnts(proxy3.get(), &refCnt, &readCnt, &writeCnt);
                REPORTER_ASSERT(reporter, 1 == refCnt);
                REPORTER_ASSERT(reporter, 0 == readCnt);
                REPORTER_ASSERT(reporter, 1 == writeCnt);

                testingOnly_getIORefCnts(proxy4.get(), &refCnt, &readCnt, &writeCnt);
                REPORTER_ASSERT(reporter, 1 == refCnt);
                REPORTER_ASSERT(reporter, 1 == readCnt);
                REPORTER_ASSERT(reporter, 1 == writeCnt);
            }

            context->flush();

            testingOnly_getIORefCnts(proxy1.get(), &refCnt, &readCnt, &writeCnt);
            REPORTER_ASSERT(reporter, 1 == refCnt);
            REPORTER_ASSERT(reporter, 0 == readCnt);
            REPORTER_ASSERT(reporter, 0 == writeCnt);

            if (texelBufferSupport) {
                testingOnly_getIORefCnts(buffer.get(), &refCnt, &readCnt, &writeCnt);
                REPORTER_ASSERT(reporter, 1 == refCnt);
                REPORTER_ASSERT(reporter, 0 == readCnt);
                REPORTER_ASSERT(reporter, 0 == writeCnt);
            }

            if (texelBufferSupport) {
                testingOnly_getIORefCnts(proxy2.get(), &refCnt, &readCnt, &writeCnt);
                REPORTER_ASSERT(reporter, 1 == refCnt);
                REPORTER_ASSERT(reporter, 0 == readCnt);
                REPORTER_ASSERT(reporter, 0 == writeCnt);

                testingOnly_getIORefCnts(proxy3.get(), &refCnt, &readCnt, &writeCnt);
                REPORTER_ASSERT(reporter, 1 == refCnt);
                REPORTER_ASSERT(reporter, 0 == readCnt);
                REPORTER_ASSERT(reporter, 0 == writeCnt);

                testingOnly_getIORefCnts(proxy4.get(), &refCnt, &readCnt, &writeCnt);
                REPORTER_ASSERT(reporter, 1 == refCnt);
                REPORTER_ASSERT(reporter, 0 == readCnt);
                REPORTER_ASSERT(reporter, 0 == writeCnt);
            }
        }
    }
}

// This test uses the random GrFragmentProcessor test factory, which relies on static initializers.
#if SK_ALLOW_STATIC_GLOBAL_INITIALIZERS

static GrColor texel_color(int i, int j) {
    SkASSERT((unsigned)i < 256 && (unsigned)j < 256);
    GrColor color = GrColorPackRGBA(j, (uint8_t)(i + j), (uint8_t)(2 * j - i), i);
    return GrPremulColor(color);
}

static GrColor4f texel_color4f(int i, int j) { return GrColor4f::FromGrColor(texel_color(i, j)); }

void test_draw_op(GrRenderTargetContext* rtc, sk_sp<GrFragmentProcessor> fp,
                  sk_sp<GrTextureProxy> inputDataProxy) {
    GrPaint paint;
    paint.addColorTextureProcessor(std::move(inputDataProxy), nullptr, SkMatrix::I());
    paint.addColorFragmentProcessor(std::move(fp));
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);

    auto op = GrRectOpFactory::MakeNonAAFill(std::move(paint), SkMatrix::I(),
                                             SkRect::MakeWH(rtc->width(), rtc->height()),
                                             GrAAType::kNone);
    rtc->addDrawOp(GrNoClip(), std::move(op));
}

#include "SkCommandLineFlags.h"
DEFINE_bool(randomProcessorTest, false, "Use non-deterministic seed for random processor tests?");

#if GR_TEST_UTILS
DEF_GPUTEST_FOR_GL_RENDERING_CONTEXTS(ProcessorOptimizationValidationTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    using FPFactory = GrProcessorTestFactory<GrFragmentProcessor>;

    uint32_t seed = 0;
    if (FLAGS_randomProcessorTest) {
        std::random_device rd;
        seed = rd();
    }
    // If a non-deterministic bot fails this test, check the output to see what seed it used, then
    // hard-code that value here:
    SkRandom random(seed);

    sk_sp<GrRenderTargetContext> rtc = context->makeDeferredRenderTargetContext(
            SkBackingFit::kExact, 256, 256, kRGBA_8888_GrPixelConfig, nullptr);
    GrSurfaceDesc desc;
    desc.fWidth = 256;
    desc.fHeight = 256;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fConfig = kRGBA_8888_GrPixelConfig;

    sk_sp<GrTextureProxy> proxies[2];

    // Put premul data into the RGBA texture that the test FPs can optionally use.
    std::unique_ptr<GrColor[]> rgbaData(new GrColor[256 * 256]);
    for (int y = 0; y < 256; ++y) {
        for (int x = 0; x < 256; ++x) {
            rgbaData.get()[256 * y + x] =
                    texel_color(random.nextULessThan(256), random.nextULessThan(256));
        }
    }
    proxies[0] = GrSurfaceProxy::MakeDeferred(context->resourceProvider(), desc, SkBudgeted::kYes,
                                              rgbaData.get(), 256 * sizeof(GrColor));

    // Put random values into the alpha texture that the test FPs can optionally use.
    desc.fConfig = kAlpha_8_GrPixelConfig;
    std::unique_ptr<uint8_t[]> alphaData(new uint8_t[256 * 256]);
    for (int y = 0; y < 256; ++y) {
        for (int x = 0; x < 256; ++x) {
            alphaData.get()[256 * y + x] = random.nextULessThan(256);
        }
    }
    proxies[1] = GrSurfaceProxy::MakeDeferred(context->resourceProvider(), desc, SkBudgeted::kYes,
                                              alphaData.get(), 256);

    if (!proxies[0] || !proxies[1]) {
        return;
    }

    GrProcessorTestData testData(&random, context, rtc.get(), proxies);

    // Use a different array of premul colors for the output of the fragment processor that preceeds
    // the fragment processor under test.
    for (int y = 0; y < 256; ++y) {
        for (int x = 0; x < 256; ++x) {
            rgbaData.get()[256 * y + x] = texel_color(x, y);
        }
    }
    desc.fConfig = kRGBA_8888_GrPixelConfig;

    sk_sp<GrTextureProxy> dataProxy = GrSurfaceProxy::MakeDeferred(context->resourceProvider(),
                                                                   desc, SkBudgeted::kYes,
                                                                   rgbaData.get(),
                                                                   256 * sizeof(GrColor));

    // Because processors factories configure themselves in random ways, this is not exhaustive.
    for (int i = 0; i < FPFactory::Count(); ++i) {
        int timesToInvokeFactory = 5;
        // Increase the number of attempts if the FP has child FPs since optimizations likely depend
        // on child optimizations being present.
        sk_sp<GrFragmentProcessor> fp = FPFactory::MakeIdx(i, &testData);
        for (int j = 0; j < fp->numChildProcessors(); ++j) {
            // This value made a reasonable trade off between time and coverage when this test was
            // written.
            timesToInvokeFactory *= FPFactory::Count() / 2;
        }
        for (int j = 0; j < timesToInvokeFactory; ++j) {
            fp = FPFactory::MakeIdx(i, &testData);
            if (!fp->instantiate(context->resourceProvider())) {
                continue;
            }

            if (!fp->hasConstantOutputForConstantInput() && !fp->preservesOpaqueInput() &&
                !fp->compatibleWithCoverageAsAlpha()) {
                continue;
            }
            test_draw_op(rtc.get(), fp, dataProxy);
            memset(rgbaData.get(), 0x0, sizeof(GrColor) * 256 * 256);
            rtc->readPixels(
                    SkImageInfo::Make(256, 256, kRGBA_8888_SkColorType, kPremul_SkAlphaType),
                    rgbaData.get(), 0, 0, 0);
            bool passing = true;
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
            for (int y = 0; y < 256 && passing; ++y) {
                for (int x = 0; x < 256 && passing; ++x) {
                    GrColor input = texel_color(x, y);
                    GrColor output = rgbaData.get()[y * 256 + x];
                    if (fp->compatibleWithCoverageAsAlpha()) {
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
                                   "Input: 0x%08x, Output: 0x%08x.",
                                   fp->name(), input, output);
                            passing = false;
                        }
                    }
                    GrColor4f input4f = texel_color4f(x, y);
                    GrColor4f output4f = GrColor4f::FromGrColor(output);
                    GrColor4f expected4f;
                    if (fp->hasConstantOutputForConstantInput(input4f, &expected4f)) {
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
                                   fp->name(), SkTMax(rDiff, SkTMax(gDiff, SkTMax(bDiff, aDiff))),
                                   kTol, input4f.fRGBA[0], input4f.fRGBA[1], input4f.fRGBA[2],
                                   input4f.fRGBA[3], output4f.fRGBA[0], output4f.fRGBA[1],
                                   output4f.fRGBA[2], output4f.fRGBA[3], expected4f.fRGBA[0],
                                   expected4f.fRGBA[1], expected4f.fRGBA[2], expected4f.fRGBA[3]);
                            passing = false;
                        }
                    }
                    if (GrColorIsOpaque(input) && fp->preservesOpaqueInput() &&
                        !GrColorIsOpaque(output)) {
                        ERRORF(reporter,
                               "Processor %s claimed opaqueness is preserved but it is not. Input: "
                               "0x%08x, Output: 0x%08x.",
                               fp->name(), input, output);
                        passing = false;
                    }
                    if (!passing) {
                        ERRORF(reporter, "Seed: 0x%08x, Processor details: %s",
                               seed, fp->dumpInfo().c_str());
                    }
                }
            }
        }
    }
}

#endif  // GR_TEST_UTILS
#endif  // SK_ALLOW_STATIC_GLOBAL_INITIALIZERS
#endif  // SK_SUPPORT_GPU
