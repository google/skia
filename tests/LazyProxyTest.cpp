/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/gpu/GrTexture.h"
#include "include/gpu/mock/GrMockTypes.h"
#include "src/core/SkExchange.h"
#include "src/core/SkMakeUnique.h"
#include "src/core/SkRectPriv.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOnFlushResourceProvider.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrRenderTargetContextPriv.h"
#include "src/gpu/GrSurfaceProxy.h"
#include "src/gpu/GrSurfaceProxyPriv.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/GrTextureProxyPriv.h"
#include "src/gpu/mock/GrMockGpu.h"

// This test verifies that lazy proxy callbacks get invoked during flush, after onFlush callbacks,
// but before Ops are executed. It also ensures that lazy proxy callbacks are invoked both for
// regular Ops and for clips.
class LazyProxyTest final : public GrOnFlushCallbackObject {
public:
    LazyProxyTest(skiatest::Reporter* reporter)
            : fReporter(reporter)
            , fHasOpTexture(false)
            , fHasClipTexture(false) {
    }

    ~LazyProxyTest() override {
        REPORTER_ASSERT(fReporter, fHasOpTexture);
        REPORTER_ASSERT(fReporter, fHasClipTexture);
    }

    void preFlush(GrOnFlushResourceProvider*, const uint32_t*, int) override {
        REPORTER_ASSERT(fReporter, !fHasOpTexture);
        REPORTER_ASSERT(fReporter, !fHasClipTexture);
    }

    void postFlush(GrDeferredUploadToken, const uint32_t* opsTaskIDs, int numOpsTaskIDs) override {
        REPORTER_ASSERT(fReporter, fHasOpTexture);
        REPORTER_ASSERT(fReporter, fHasClipTexture);
    }

    class Op final : public GrDrawOp {
    public:
        DEFINE_OP_CLASS_ID

        static std::unique_ptr<GrDrawOp> Make(GrRecordingContext* context,
                                              GrProxyProvider* proxyProvider,
                                              LazyProxyTest* test,
                                              bool nullTexture) {
            GrOpMemoryPool* pool = context->priv().opMemoryPool();

            return pool->allocate<Op>(context, proxyProvider, test, nullTexture);
        }

        void visitProxies(const VisitProxyFunc& func) const override {
            func(fProxy.get(), GrMipMapped::kNo);
        }

        void onExecute(GrOpFlushState*, const SkRect& chainBounds) override {
            REPORTER_ASSERT(fTest->fReporter, fTest->fHasOpTexture);
            REPORTER_ASSERT(fTest->fReporter, fTest->fHasClipTexture);
        }

    private:
        friend class GrOpMemoryPool; // for ctor

        Op(GrRecordingContext* ctx, GrProxyProvider* proxyProvider,
           LazyProxyTest* test, bool nullTexture)
                    : GrDrawOp(ClassID()), fTest(test) {
            const GrBackendFormat format =
                ctx->priv().caps()->getDefaultBackendFormat(GrColorType::kBGR_565,
                                                            GrRenderable::kNo);
            fProxy = GrProxyProvider::MakeFullyLazyProxy(
                    [this, format,
                     nullTexture](GrResourceProvider* rp) -> GrSurfaceProxy::LazyCallbackResult {
                        REPORTER_ASSERT(fTest->fReporter, !fTest->fHasOpTexture);
                        fTest->fHasOpTexture = true;
                        if (nullTexture) {
                            return {};
                        } else {
                            GrSurfaceDesc desc;
                            desc.fWidth = 1234;
                            desc.fHeight = 567;
                            desc.fConfig = kRGB_565_GrPixelConfig;
                            sk_sp<GrTexture> texture = rp->createTexture(
                                    desc, format, GrRenderable::kNo, 1, GrMipMapped::kNo,
                                    SkBudgeted::kYes, GrProtected::kNo);
                            REPORTER_ASSERT(fTest->fReporter, texture);
                            return texture;
                        }
                    },
                    format, GrRenderable::kNo, 1, GrProtected::kNo, kTopLeft_GrSurfaceOrigin,
                    kRGB_565_GrPixelConfig, *proxyProvider->caps(),
                    GrSurfaceProxy::UseAllocator::kYes);

            this->setBounds(SkRectPriv::MakeLargest(), GrOp::HasAABloat::kNo,
                            GrOp::IsZeroArea::kNo);
        }

        const char* name() const override { return "LazyProxyTest::Op"; }
        FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }
        GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip* clip,
                                          bool hasMixedSampledCoverage, GrClampType) override {
            return GrProcessorSet::EmptySetAnalysis();
        }
        void onPrepare(GrOpFlushState*) override {}

        LazyProxyTest* const fTest;
        sk_sp<GrTextureProxy> fProxy;
    };

    class ClipFP : public GrFragmentProcessor {
    public:
        ClipFP(GrRecordingContext* ctx, GrProxyProvider* proxyProvider, LazyProxyTest* test,
               GrTextureProxy* atlas)
                : GrFragmentProcessor(kTestFP_ClassID, kNone_OptimizationFlags)
                , fContext(ctx)
                , fProxyProvider(proxyProvider)
                , fTest(test)
                , fAtlas(atlas) {
            const GrBackendFormat format =
                ctx->priv().caps()->getDefaultBackendFormat(GrColorType::kAlpha_F16,
                                                            GrRenderable::kYes);
            fLazyProxy = GrProxyProvider::MakeFullyLazyProxy(
                    [this](GrResourceProvider* rp) -> GrSurfaceProxy::LazyCallbackResult {
                        REPORTER_ASSERT(fTest->fReporter, !fTest->fHasClipTexture);
                        fTest->fHasClipTexture = true;
                        fAtlas->instantiate(rp);
                        return sk_ref_sp(fAtlas->peekTexture());
                    },
                    format, GrRenderable::kYes, 1, GrProtected::kNo, kBottomLeft_GrSurfaceOrigin,
                    kAlpha_half_GrPixelConfig, *proxyProvider->caps(),
                    GrSurfaceProxy::UseAllocator::kYes);
            fAccess.reset(fLazyProxy, GrSamplerState::Filter::kNearest,
                          GrSamplerState::WrapMode::kClamp);
            this->setTextureSamplerCnt(1);
        }

    private:
        const char* name() const override { return "LazyProxyTest::ClipFP"; }
        std::unique_ptr<GrFragmentProcessor> clone() const override {
            return skstd::make_unique<ClipFP>(fContext, fProxyProvider, fTest, fAtlas);
        }
        GrGLSLFragmentProcessor* onCreateGLSLInstance() const override { return nullptr; }
        void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {}
        bool onIsEqual(const GrFragmentProcessor&) const override { return false; }
        const TextureSampler& onTextureSampler(int) const override { return fAccess; }

        GrRecordingContext* const fContext;
        GrProxyProvider* const fProxyProvider;
        LazyProxyTest* const fTest;
        GrTextureProxy* const fAtlas;
        sk_sp<GrTextureProxy> fLazyProxy;
        TextureSampler fAccess;
    };


    class Clip : public GrClip {
    public:
        Clip(LazyProxyTest* test, GrTextureProxy* atlas)
                : fTest(test)
                , fAtlas(atlas) {}

    private:
        bool apply(GrRecordingContext* context, GrRenderTargetContext*, bool useHWAA,
                   bool hasUserStencilSettings, GrAppliedClip* out, SkRect* bounds) const override {
            GrProxyProvider* proxyProvider = context->priv().proxyProvider();
            out->addCoverageFP(skstd::make_unique<ClipFP>(context, proxyProvider, fTest, fAtlas));
            return true;
        }
        bool quickContains(const SkRect&) const final { return false; }
        bool isRRect(const SkRect& rtBounds, SkRRect* rr, GrAA*) const final { return false; }
        void getConservativeBounds(int width, int height, SkIRect* rect, bool* iior) const final {
            rect->setLTRB(0, 0, width, height);
            if (iior) {
                *iior = false;
            }
        }

        LazyProxyTest* const fTest;
        GrTextureProxy* fAtlas;
    };

private:
    skiatest::Reporter* fReporter;
    bool fHasOpTexture;
    bool fHasClipTexture;
};

DEF_GPUTEST(LazyProxyTest, reporter, /* options */) {
    GrMockOptions mockOptions;
    mockOptions.fConfigOptions[(int)GrColorType::kAlpha_F16].fRenderability =
            GrMockOptions::ConfigOptions::Renderability::kNonMSAA;
    mockOptions.fConfigOptions[(int)GrColorType::kAlpha_F16].fTexturable = true;
    sk_sp<GrContext> ctx = GrContext::MakeMock(&mockOptions, GrContextOptions());
    GrProxyProvider* proxyProvider = ctx->priv().proxyProvider();
    for (bool nullTexture : {false, true}) {
        LazyProxyTest test(reporter);
        ctx->priv().addOnFlushCallbackObject(&test);
        auto rtc = ctx->priv().makeDeferredRenderTargetContext(SkBackingFit::kExact, 100, 100,
                                                               GrColorType::kRGBA_8888, nullptr);
        REPORTER_ASSERT(reporter, rtc);
        auto mockAtlas = ctx->priv().makeDeferredRenderTargetContext(
                SkBackingFit::kExact, 10, 10, GrColorType::kAlpha_F16, nullptr);
        REPORTER_ASSERT(reporter, mockAtlas);
        rtc->priv().testingOnly_addDrawOp(LazyProxyTest::Clip(&test, mockAtlas->asTextureProxy()),
                        LazyProxyTest::Op::Make(ctx.get(), proxyProvider, &test, nullTexture));
        ctx->priv().testingOnly_flushAndRemoveOnFlushCallbackObject(&test);
    }
}

static const int kSize = 16;

DEF_GPUTEST(LazyProxyReleaseTest, reporter, /* options */) {
    GrMockOptions mockOptions;
    sk_sp<GrContext> ctx = GrContext::MakeMock(&mockOptions, GrContextOptions());
    auto proxyProvider = ctx->priv().proxyProvider();
    const GrCaps* caps = ctx->priv().caps();

    GrSurfaceDesc desc;
    desc.fWidth = kSize;
    desc.fHeight = kSize;
    desc.fConfig = kRGBA_8888_GrPixelConfig;

    GrBackendFormat format = caps->getDefaultBackendFormat(GrColorType::kRGBA_8888,
                                                           GrRenderable::kNo);

    auto tex = ctx->priv().resourceProvider()->createTexture(desc, format, GrRenderable::kNo, 1,
                                                             GrMipMapped::kNo, SkBudgeted::kNo,
                                                             GrProtected::kNo);
    using LazyInstantiationResult = GrSurfaceProxy::LazyCallbackResult;
    for (bool doInstantiate : {true, false}) {
        for (bool releaseCallback : {false, true}) {
            int testCount = 0;
            // Sets an integer to 1 when the callback is called and -1 when it is deleted.
            class TestCallback {
            public:
                TestCallback(int* value, bool releaseCallback, sk_sp<GrTexture> tex)
                        : fValue(value)
                        , fReleaseCallback(releaseCallback)
                        , fTexture(std::move(tex)) {}
                TestCallback(const TestCallback& that) { SkASSERT(0); }
                TestCallback(TestCallback&& that)
                        : fValue(that.fValue)
                        , fReleaseCallback(that.fReleaseCallback)
                        , fTexture(std::move(that.fTexture)) {
                    that.fValue = nullptr;
                }

                ~TestCallback() { fValue ? (void)(*fValue = -1) : void(); }

                TestCallback& operator=(TestCallback&& that) {
                    fValue = skstd::exchange(that.fValue, nullptr);
                    return *this;
                }
                TestCallback& operator=(const TestCallback& that) = delete;

                LazyInstantiationResult operator()(GrResourceProvider* resourceProvider) const {
                    *fValue = 1;
                    return {fTexture, fReleaseCallback};
                }

            private:
                int* fValue = nullptr;
                bool fReleaseCallback;
                sk_sp<GrTexture> fTexture;
            };
            sk_sp<GrTextureProxy> proxy = proxyProvider->createLazyProxy(
                    TestCallback(&testCount, releaseCallback, tex), format, desc, GrRenderable::kNo,
                    1, kTopLeft_GrSurfaceOrigin, GrMipMapped::kNo, GrMipMapsStatus::kNotAllocated,
                    GrInternalSurfaceFlags::kNone, SkBackingFit::kExact, SkBudgeted::kNo,
                    GrProtected::kNo, GrSurfaceProxy::UseAllocator::kYes);

            REPORTER_ASSERT(reporter, proxy.get());
            REPORTER_ASSERT(reporter, 0 == testCount);

            if (doInstantiate) {
                proxy->priv().doLazyInstantiation(ctx->priv().resourceProvider());
                if (releaseCallback) {
                    // We will call the cleanup and delete the callback in the
                    // doLazyInstantiationCall.
                    REPORTER_ASSERT(reporter, -1 == testCount);
                } else {
                    REPORTER_ASSERT(reporter, 1 == testCount);
                }
                proxy.reset();
                REPORTER_ASSERT(reporter, -1 == testCount);
            } else {
                proxy.reset();
                REPORTER_ASSERT(reporter, -1 == testCount);
            }
        }
    }
}

class LazyFailedInstantiationTestOp : public GrDrawOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrDrawOp> Make(GrContext* context,
                                          GrProxyProvider* proxyProvider,
                                          int* testExecuteValue,
                                          bool shouldFailInstantiation) {
        GrOpMemoryPool* pool = context->priv().opMemoryPool();

        return pool->allocate<LazyFailedInstantiationTestOp>(context, proxyProvider,
                                                             testExecuteValue,
                                                             shouldFailInstantiation);
    }

    void visitProxies(const VisitProxyFunc& func) const override {
        func(fLazyProxy.get(), GrMipMapped::kNo);
    }

private:
    friend class GrOpMemoryPool; // for ctor

    LazyFailedInstantiationTestOp(GrContext* ctx, GrProxyProvider* proxyProvider,
                                  int* testExecuteValue, bool shouldFailInstantiation)
            : INHERITED(ClassID())
            , fTestExecuteValue(testExecuteValue) {
        GrSurfaceDesc desc;
        desc.fWidth = kSize;
        desc.fHeight = kSize;
        desc.fConfig = kRGBA_8888_GrPixelConfig;
        GrBackendFormat format =
            ctx->priv().caps()->getDefaultBackendFormat(GrColorType::kRGBA_8888,
                                                        GrRenderable::kNo);

        fLazyProxy = proxyProvider->createLazyProxy(
                [testExecuteValue, shouldFailInstantiation, desc,
                 format](GrResourceProvider* rp) -> GrSurfaceProxy::LazyCallbackResult {
                    if (shouldFailInstantiation) {
                        *testExecuteValue = 1;
                        return {};
                    }
                    return {rp->createTexture(desc, format, GrRenderable::kNo, 1, GrMipMapped::kNo,
                                              SkBudgeted::kNo, GrProtected::kNo),
                            true, GrSurfaceProxy::LazyInstantiationKeyMode::kUnsynced};
                },
                format, desc, GrRenderable::kNo, 1, kTopLeft_GrSurfaceOrigin, GrMipMapped::kNo,
                GrMipMapsStatus::kNotAllocated, GrInternalSurfaceFlags::kNone, SkBackingFit::kExact,
                SkBudgeted::kNo, GrProtected::kNo, GrSurfaceProxy::UseAllocator::kYes);

        SkASSERT(fLazyProxy.get());

        this->setBounds(SkRect::MakeIWH(kSize, kSize),
                        HasAABloat::kNo, IsZeroArea::kNo);
    }

    const char* name() const override { return "LazyFailedInstantiationTestOp"; }
    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }
    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*,
                                      bool hasMixedSampledCoverage, GrClampType) override {
        return GrProcessorSet::EmptySetAnalysis();
    }
    void onPrepare(GrOpFlushState*) override {}
    void onExecute(GrOpFlushState* state, const SkRect& chainBounds) override {
        *fTestExecuteValue = 2;
    }

    int* fTestExecuteValue;
    sk_sp<GrTextureProxy> fLazyProxy;

    typedef GrDrawOp INHERITED;
};

// Test that when a lazy proxy fails to instantiate during flush that we drop the Op that it was
// associated with.
DEF_GPUTEST(LazyProxyFailedInstantiationTest, reporter, /* options */) {
    GrMockOptions mockOptions;
    sk_sp<GrContext> ctx = GrContext::MakeMock(&mockOptions, GrContextOptions());
    GrProxyProvider* proxyProvider = ctx->priv().proxyProvider();
    for (bool failInstantiation : {false, true}) {
        auto rtc = ctx->priv().makeDeferredRenderTargetContext(SkBackingFit::kExact, 100, 100,
                                                               GrColorType::kRGBA_8888, nullptr);
        REPORTER_ASSERT(reporter, rtc);

        rtc->clear(nullptr, SkPMColor4f::FromBytes_RGBA(0xbaaaaaad),
                   GrRenderTargetContext::CanClearFullscreen::kYes);

        int executeTestValue = 0;
        rtc->priv().testingOnly_addDrawOp(LazyFailedInstantiationTestOp::Make(
                ctx.get(), proxyProvider, &executeTestValue, failInstantiation));
        ctx->flush();

        if (failInstantiation) {
            REPORTER_ASSERT(reporter, 1 == executeTestValue);
        } else {
            REPORTER_ASSERT(reporter, 2 == executeTestValue);
        }
    }
}
