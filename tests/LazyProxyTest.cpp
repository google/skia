/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/gpu/GrTexture.h"
#include "include/gpu/mock/GrMockTypes.h"
#include "include/private/GrSurfaceProxy.h"
#include "include/private/GrTextureProxy.h"
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
#include "src/gpu/GrSurfaceProxyPriv.h"
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

    void preFlush(GrOnFlushResourceProvider*, const uint32_t*, int,
                  SkTArray<sk_sp<GrRenderTargetContext>>*) override {
        REPORTER_ASSERT(fReporter, !fHasOpTexture);
        REPORTER_ASSERT(fReporter, !fHasClipTexture);
    }

    void postFlush(GrDeferredUploadToken, const uint32_t* opListIDs, int numOpListIDs) override {
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

        void visitProxies(const VisitProxyFunc& func, VisitorType) const override {
            func(fProxy.get());
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
                    ctx->priv().caps()->getBackendFormatFromColorType(kRGB_565_SkColorType);
            fProxy = GrProxyProvider::MakeFullyLazyProxy(
                    [this, nullTexture](
                            GrResourceProvider* rp) -> GrSurfaceProxy::LazyInstantiationResult {
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
                                desc, SkBudgeted::kYes, GrResourceProvider::Flags::kNoPendingIO);
                            REPORTER_ASSERT(fTest->fReporter, texture);
                            return std::move(texture);
                        }
                    },
                    format, GrProxyProvider::Renderable::kNo, kTopLeft_GrSurfaceOrigin,
                    kRGB_565_GrPixelConfig, *proxyProvider->caps());

            this->setBounds(SkRectPriv::MakeLargest(), GrOp::HasAABloat::kNo,
                            GrOp::IsZeroArea::kNo);
        }

        const char* name() const override { return "LazyProxyTest::Op"; }
        FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }
        GrProcessorSet::Analysis finalize(
                const GrCaps&, const GrAppliedClip* clip, GrFSAAType, GrClampType) override {
            if (clip) {
                for (int i = 0; i < clip->numClipCoverageFragmentProcessors(); ++i) {
                    const GrFragmentProcessor* clipFP = clip->clipCoverageFragmentProcessor(i);
                    clipFP->markPendingExecution();
                }
            }
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
                ctx->priv().caps()->getBackendFormatFromGrColorType(GrColorType::kAlpha_F16,
                                                                    GrSRGBEncoded::kNo);
            fLazyProxy = GrProxyProvider::MakeFullyLazyProxy(
                    [this](GrResourceProvider* rp) -> GrSurfaceProxy::LazyInstantiationResult {
                        REPORTER_ASSERT(fTest->fReporter, !fTest->fHasClipTexture);
                        fTest->fHasClipTexture = true;
                        fAtlas->instantiate(rp);
                        return sk_ref_sp(fAtlas->peekTexture());
                    },
                    format, GrProxyProvider::Renderable::kYes, kBottomLeft_GrSurfaceOrigin,
                    kAlpha_half_GrPixelConfig, *proxyProvider->caps());
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
            rect->set(0, 0, width, height);
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
    mockOptions.fConfigOptions[kAlpha_half_GrPixelConfig].fRenderability =
            GrMockOptions::ConfigOptions::Renderability::kNonMSAA;
    mockOptions.fConfigOptions[kAlpha_half_GrPixelConfig].fTexturable = true;
    sk_sp<GrContext> ctx = GrContext::MakeMock(&mockOptions, GrContextOptions());
    GrProxyProvider* proxyProvider = ctx->priv().proxyProvider();
    for (bool nullTexture : {false, true}) {
        LazyProxyTest test(reporter);
        ctx->priv().addOnFlushCallbackObject(&test);
        GrBackendFormat format =
            ctx->priv().caps()->getBackendFormatFromColorType(kRGBA_8888_SkColorType);
        sk_sp<GrRenderTargetContext> rtc = ctx->priv().makeDeferredRenderTargetContext(
                                                             format, SkBackingFit::kExact, 100, 100,
                                                             kRGBA_8888_GrPixelConfig, nullptr);
        REPORTER_ASSERT(reporter, rtc);
        format =
                ctx->priv().caps()->getBackendFormatFromGrColorType(GrColorType::kAlpha_F16,
                                                                           GrSRGBEncoded::kNo);
        sk_sp<GrRenderTargetContext> mockAtlas = ctx->priv().makeDeferredRenderTargetContext(
                                                     format, SkBackingFit::kExact, 10, 10,
                                                     kAlpha_half_GrPixelConfig, nullptr);
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

    GrSurfaceDesc desc;
    desc.fWidth = kSize;
    desc.fHeight = kSize;
    desc.fConfig = kRGBA_8888_GrPixelConfig;

    GrBackendFormat format =
            ctx->priv().caps()->getBackendFormatFromColorType(kRGBA_8888_SkColorType);

    using LazyInstantiationType = GrSurfaceProxy::LazyInstantiationType;
    using LazyInstantiationResult = GrSurfaceProxy::LazyInstantiationResult;
    for (bool doInstantiate : {true, false}) {
        for (auto lazyType : {LazyInstantiationType::kSingleUse,
                              LazyInstantiationType::kMultipleUse,
                              LazyInstantiationType::kDeinstantiate}) {
            int testCount = 0;
            // Sets an integer to 1 when the callback is called and -1 when it is deleted.
            class TestCallback {
            public:
                TestCallback(int* value) : fValue(value) {}
                TestCallback(const TestCallback& that) { SkASSERT(0); }
                TestCallback(TestCallback&& that) : fValue(that.fValue) { that.fValue = nullptr; }

                ~TestCallback() { fValue ? (void)(*fValue = -1) : void(); }

                TestCallback& operator=(TestCallback&& that) {
                    fValue = skstd::exchange(that.fValue, nullptr);
                    return *this;
                }
                TestCallback& operator=(const TestCallback& that) = delete;

                LazyInstantiationResult operator()(GrResourceProvider* resourceProvider) const {
                    *fValue = 1;
                    return {};
                }

            private:
                int* fValue = nullptr;
            };
            sk_sp<GrTextureProxy> proxy = proxyProvider->createLazyProxy(
                    TestCallback(&testCount), format, desc, kTopLeft_GrSurfaceOrigin,
                    GrMipMapped::kNo, GrInternalSurfaceFlags::kNone, SkBackingFit::kExact,
                    SkBudgeted::kNo, lazyType);

            REPORTER_ASSERT(reporter, proxy.get());
            REPORTER_ASSERT(reporter, 0 == testCount);

            if (doInstantiate) {
                proxy->priv().doLazyInstantiation(ctx->priv().resourceProvider());
                if (LazyInstantiationType::kSingleUse == proxy->priv().lazyInstantiationType()) {
                    // In SingleUse we will call the cleanup and delete the callback in the
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

    void visitProxies(const VisitProxyFunc& func, VisitorType) const override {
        func(fLazyProxy.get());
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
                ctx->priv().caps()->getBackendFormatFromColorType(kRGBA_8888_SkColorType);

        fLazyProxy = proxyProvider->createLazyProxy(
                [testExecuteValue, shouldFailInstantiation,
                 desc](GrResourceProvider* rp) -> GrSurfaceProxy::LazyInstantiationResult {
                    if (shouldFailInstantiation) {
                        *testExecuteValue = 1;
                        return {};
                    }
                    return {rp->createTexture(desc, SkBudgeted::kNo,
                                              GrResourceProvider::Flags::kNoPendingIO),
                            GrSurfaceProxy::LazyInstantiationKeyMode::kUnsynced};
                },
                format, desc, kTopLeft_GrSurfaceOrigin, GrMipMapped::kNo, SkBackingFit::kExact,
                SkBudgeted::kNo);

        SkASSERT(fLazyProxy.get());

        this->setBounds(SkRect::MakeIWH(kSize, kSize),
                        HasAABloat::kNo, IsZeroArea::kNo);
    }

    const char* name() const override { return "LazyFailedInstantiationTestOp"; }
    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }
    GrProcessorSet::Analysis finalize(
            const GrCaps&, const GrAppliedClip*, GrFSAAType, GrClampType) override {
        return GrProcessorSet::EmptySetAnalysis();
    }
    void onPrepare(GrOpFlushState*) override {}
    void onExecute(GrOpFlushState* state, const SkRect& chainBounds) override {
        *fTestExecuteValue = 2;
    }

    int* fTestExecuteValue;
    sk_sp<GrSurfaceProxy> fLazyProxy;

    typedef GrDrawOp INHERITED;
};

// Test that when a lazy proxy fails to instantiate during flush that we drop the Op that it was
// associated with.
DEF_GPUTEST(LazyProxyFailedInstantiationTest, reporter, /* options */) {
    GrMockOptions mockOptions;
    sk_sp<GrContext> ctx = GrContext::MakeMock(&mockOptions, GrContextOptions());
    GrProxyProvider* proxyProvider = ctx->priv().proxyProvider();
    GrBackendFormat format =
                ctx->priv().caps()->getBackendFormatFromColorType(kRGBA_8888_SkColorType);
    for (bool failInstantiation : {false, true}) {
        sk_sp<GrRenderTargetContext> rtc = ctx->priv().makeDeferredRenderTargetContext(
                                                     format, SkBackingFit::kExact, 100, 100,
                                                     kRGBA_8888_GrPixelConfig, nullptr);
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

class LazyDeinstantiateTestOp : public GrDrawOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrDrawOp> Make(GrContext* context, sk_sp<GrTextureProxy> proxy) {
        GrOpMemoryPool* pool = context->priv().opMemoryPool();

        return pool->allocate<LazyDeinstantiateTestOp>(std::move(proxy));
    }

    void visitProxies(const VisitProxyFunc& func, VisitorType) const override {
        func(fLazyProxy.get());
    }

private:
    friend class GrOpMemoryPool; // for ctor

    LazyDeinstantiateTestOp(sk_sp<GrTextureProxy> proxy)
            : INHERITED(ClassID()), fLazyProxy(std::move(proxy)) {
        this->setBounds(SkRect::MakeIWH(kSize, kSize), HasAABloat::kNo, IsZeroArea::kNo);
    }

    const char* name() const override { return "LazyDeinstantiateTestOp"; }
    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }
    GrProcessorSet::Analysis finalize(
            const GrCaps&, const GrAppliedClip*, GrFSAAType, GrClampType) override {
        return GrProcessorSet::EmptySetAnalysis();
    }
    void onPrepare(GrOpFlushState*) override {}
    void onExecute(GrOpFlushState* state, const SkRect& chainBounds) override {}

    sk_sp<GrSurfaceProxy> fLazyProxy;

    typedef GrDrawOp INHERITED;
};

static void DeinstantiateReleaseProc(void* releaseValue) { (*static_cast<int*>(releaseValue))++; }

// Test that lazy proxies with the Deinstantiate LazyCallbackType are deinstantiated and released as
// expected.
DEF_GPUTEST(LazyProxyDeinstantiateTest, reporter, /* options */) {
    GrMockOptions mockOptions;
    sk_sp<GrContext> ctx = GrContext::MakeMock(&mockOptions, GrContextOptions());
    GrProxyProvider* proxyProvider = ctx->priv().proxyProvider();
    GrGpu* gpu = ctx->priv().getGpu();

    GrBackendFormat format =
                ctx->priv().caps()->getBackendFormatFromColorType(kRGBA_8888_SkColorType);

    using LazyType = GrSurfaceProxy::LazyInstantiationType;
    for (auto lazyType : {LazyType::kSingleUse, LazyType::kMultipleUse, LazyType::kDeinstantiate}) {
        sk_sp<GrRenderTargetContext> rtc = ctx->priv().makeDeferredRenderTargetContext(
                format, SkBackingFit::kExact, 100, 100,
                kRGBA_8888_GrPixelConfig, nullptr);
        REPORTER_ASSERT(reporter, rtc);

        rtc->clear(nullptr, SkPMColor4f::FromBytes_RGBA(0xbaaaaaad),
                   GrRenderTargetContext::CanClearFullscreen::kYes);

        int instantiateTestValue = 0;
        int releaseTestValue = 0;
        int* instantiatePtr = &instantiateTestValue;
        int* releasePtr = &releaseTestValue;
        GrSurfaceDesc desc;
        desc.fWidth = kSize;
        desc.fHeight = kSize;
        desc.fConfig = kRGBA_8888_GrPixelConfig;

        GrBackendTexture backendTex = gpu->createTestingOnlyBackendTexture(
                nullptr, kSize, kSize, GrColorType::kRGBA_8888, false, GrMipMapped::kNo);

        sk_sp<GrTextureProxy> lazyProxy = proxyProvider->createLazyProxy(
                [instantiatePtr, releasePtr,
                 backendTex](GrResourceProvider* rp) -> GrSurfaceProxy::LazyInstantiationResult {
                    sk_sp<GrTexture> texture =
                            rp->wrapBackendTexture(backendTex, kBorrow_GrWrapOwnership,
                                                   GrWrapCacheable::kNo, kRead_GrIOType);
                    if (!texture) {
                        return {};
                    }
                    (*instantiatePtr)++;
                    texture->setRelease(DeinstantiateReleaseProc, releasePtr);
                    return std::move(texture);
                },
                format, desc, kTopLeft_GrSurfaceOrigin, GrMipMapped::kNo,
                GrInternalSurfaceFlags::kReadOnly, SkBackingFit::kExact, SkBudgeted::kNo, lazyType);

        REPORTER_ASSERT(reporter, lazyProxy.get());

        rtc->priv().testingOnly_addDrawOp(LazyDeinstantiateTestOp::Make(ctx.get(), lazyProxy));

        ctx->flush();

        REPORTER_ASSERT(reporter, 1 == instantiateTestValue);
        if (LazyType::kDeinstantiate == lazyType) {
            REPORTER_ASSERT(reporter, 1 == releaseTestValue);
        } else {
            REPORTER_ASSERT(reporter, 0 == releaseTestValue);
        }

        // This should cause the uninstantiate proxies to be instantiated again but have no effect
        // on the others
        rtc->priv().testingOnly_addDrawOp(LazyDeinstantiateTestOp::Make(ctx.get(), lazyProxy));
        // Add a second op to make sure we only instantiate once.
        rtc->priv().testingOnly_addDrawOp(LazyDeinstantiateTestOp::Make(ctx.get(), lazyProxy));
        ctx->flush();

        if (LazyType::kDeinstantiate == lazyType) {
            REPORTER_ASSERT(reporter, 2 == instantiateTestValue);
            REPORTER_ASSERT(reporter, 2 == releaseTestValue);
        } else {
            REPORTER_ASSERT(reporter, 1 == instantiateTestValue);
            REPORTER_ASSERT(reporter, 0 == releaseTestValue);
        }

        lazyProxy.reset();
        if (LazyType::kDeinstantiate == lazyType) {
            REPORTER_ASSERT(reporter, 2 == releaseTestValue);
        } else {
            REPORTER_ASSERT(reporter, 1 == releaseTestValue);
        }

        gpu->deleteTestingOnlyBackendTexture(backendTex);
    }
}
