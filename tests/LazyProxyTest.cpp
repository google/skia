/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#include "GrClip.h"
#include "GrContextPriv.h"
#include "GrMemoryPool.h"
#include "GrOnFlushResourceProvider.h"
#include "GrProxyProvider.h"
#include "GrRenderTargetContext.h"
#include "GrRenderTargetContextPriv.h"
#include "GrSurfaceProxy.h"
#include "GrSurfaceProxyPriv.h"
#include "GrTexture.h"
#include "GrTextureProxy.h"
#include "GrTextureProxyPriv.h"
#include "SkMakeUnique.h"
#include "SkRectPriv.h"
#include "mock/GrMockGpu.h"
#include "mock/GrMockTypes.h"

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

        static std::unique_ptr<GrDrawOp> Make(GrContext* context,
                                              GrProxyProvider* proxyProvider,
                                              LazyProxyTest* test,
                                              bool nullTexture) {
            GrOpMemoryPool* pool = context->contextPriv().opMemoryPool();

            return pool->allocate<Op>(proxyProvider, test, nullTexture);
        }

        void visitProxies(const VisitProxyFunc& func) const override {
            func(fProxy.get());
        }

        void onExecute(GrOpFlushState*) override {
            REPORTER_ASSERT(fTest->fReporter, fTest->fHasOpTexture);
            REPORTER_ASSERT(fTest->fReporter, fTest->fHasClipTexture);
        }

    private:
        friend class GrOpMemoryPool; // for ctor

        Op(GrProxyProvider* proxyProvider, LazyProxyTest* test, bool nullTexture)
                    : GrDrawOp(ClassID()), fTest(test) {
            fProxy = GrProxyProvider::MakeFullyLazyProxy(
                    [this, nullTexture](GrResourceProvider* rp) {
                        if (!rp) {
                            return sk_sp<GrTexture>();
                        }
                        REPORTER_ASSERT(fTest->fReporter, !fTest->fHasOpTexture);
                        fTest->fHasOpTexture = true;
                        if (nullTexture) {
                            return sk_sp<GrTexture>();
                        } else {
                            GrSurfaceDesc desc;
                            desc.fWidth = 1234;
                            desc.fHeight = 567;
                            desc.fConfig = kRGB_565_GrPixelConfig;
                            sk_sp<GrTexture> texture = rp->createTexture(desc, SkBudgeted::kYes);
                            REPORTER_ASSERT(fTest->fReporter, texture);
                            return texture;
                        }
                    },
                    GrProxyProvider::Renderable::kNo, kTopLeft_GrSurfaceOrigin,
                    kRGB_565_GrPixelConfig, *proxyProvider->caps());

            this->setBounds(SkRectPriv::MakeLargest(), GrOp::HasAABloat::kNo,
                            GrOp::IsZeroArea::kNo);
        }

        const char* name() const override { return "LazyProxyTest::Op"; }
        FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }
        RequiresDstTexture finalize(const GrCaps&, const GrAppliedClip*) override {
            return RequiresDstTexture::kNo;
        }
        bool onCombineIfPossible(GrOp* other, const GrCaps& caps) override { return false; }
        void onPrepare(GrOpFlushState*) override {}

        LazyProxyTest* const fTest;
        sk_sp<GrTextureProxy> fProxy;
    };

    class ClipFP : public GrFragmentProcessor {
    public:
        ClipFP(GrProxyProvider* proxyProvider, LazyProxyTest* test, GrTextureProxy* atlas)
                : GrFragmentProcessor(kTestFP_ClassID, kNone_OptimizationFlags)
                , fProxyProvider(proxyProvider)
                , fTest(test)
                , fAtlas(atlas) {
            fLazyProxy = GrProxyProvider::MakeFullyLazyProxy(
                                [this](GrResourceProvider* rp) {
                                    if (!rp) {
                                        return sk_sp<GrTexture>();
                                    }
                                    REPORTER_ASSERT(fTest->fReporter, !fTest->fHasClipTexture);
                                    fTest->fHasClipTexture = true;
                                    fAtlas->instantiate(rp);
                                    return sk_ref_sp(fAtlas->priv().peekTexture());
                                },
                                GrProxyProvider::Renderable::kYes,
                                kBottomLeft_GrSurfaceOrigin,
                                kAlpha_half_GrPixelConfig, *proxyProvider->caps());
            fAccess.reset(fLazyProxy, GrSamplerState::Filter::kNearest,
                          GrSamplerState::WrapMode::kClamp, kFragment_GrShaderFlag);
            this->addTextureSampler(&fAccess);
        }

    private:
        const char* name() const override { return "LazyProxyTest::ClipFP"; }
        std::unique_ptr<GrFragmentProcessor> clone() const override {
            return skstd::make_unique<ClipFP>(fProxyProvider, fTest, fAtlas);
        }
        GrGLSLFragmentProcessor* onCreateGLSLInstance() const override { return nullptr; }
        void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {}
        bool onIsEqual(const GrFragmentProcessor&) const override { return false; }

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
        bool apply(GrContext* context, GrRenderTargetContext*, bool, bool, GrAppliedClip* out,
                   SkRect* bounds) const override {
            GrProxyProvider* proxyProvider = context->contextPriv().proxyProvider();
            out->addCoverageFP(skstd::make_unique<ClipFP>(proxyProvider, fTest, fAtlas));
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
    GrProxyProvider* proxyProvider = ctx->contextPriv().proxyProvider();
    for (bool nullTexture : {false, true}) {
        LazyProxyTest test(reporter);
        ctx->contextPriv().addOnFlushCallbackObject(&test);
        sk_sp<GrRenderTargetContext> rtc = ctx->contextPriv().makeDeferredRenderTargetContext(
                                                             SkBackingFit::kExact, 100, 100,
                                                             kRGBA_8888_GrPixelConfig, nullptr);
        REPORTER_ASSERT(reporter, rtc);
        sk_sp<GrRenderTargetContext> mockAtlas = ctx->contextPriv().makeDeferredRenderTargetContext(
                                                     SkBackingFit::kExact, 10, 10,
                                                     kAlpha_half_GrPixelConfig, nullptr);
        REPORTER_ASSERT(reporter, mockAtlas);
        rtc->priv().testingOnly_addDrawOp(LazyProxyTest::Clip(&test, mockAtlas->asTextureProxy()),
                        LazyProxyTest::Op::Make(ctx.get(), proxyProvider, &test, nullTexture));
        ctx->contextPriv().testingOnly_flushAndRemoveOnFlushCallbackObject(&test);
    }
}

static const int kSize = 16;

DEF_GPUTEST(LazyProxyReleaseTest, reporter, /* options */) {
    GrMockOptions mockOptions;
    sk_sp<GrContext> ctx = GrContext::MakeMock(&mockOptions, GrContextOptions());
    auto proxyProvider = ctx->contextPriv().proxyProvider();

    GrSurfaceDesc desc;
    desc.fWidth = kSize;
    desc.fHeight = kSize;
    desc.fConfig = kRGBA_8888_GrPixelConfig;

    using LazyInstantiationType = GrSurfaceProxy::LazyInstantiationType;
    for (bool doInstantiate : {true, false}) {
        for (auto lazyType : {LazyInstantiationType::kSingleUse,
                              LazyInstantiationType::kMultipleUse,
                              LazyInstantiationType::kUninstantiate}) {
            int testCount = 0;
            int* testCountPtr = &testCount;
            sk_sp<GrTextureProxy> proxy = proxyProvider->createLazyProxy(
                    [testCountPtr](GrResourceProvider* resourceProvider) {
                        if (!resourceProvider) {
                            *testCountPtr = -1;
                            return sk_sp<GrTexture>();
                        }
                        *testCountPtr = 1;
                        return sk_sp<GrTexture>();
                    },
                    desc, kTopLeft_GrSurfaceOrigin, GrMipMapped::kNo, GrInternalSurfaceFlags::kNone,
                    SkBackingFit::kExact, SkBudgeted::kNo, lazyType);

            REPORTER_ASSERT(reporter, proxy.get());
            REPORTER_ASSERT(reporter, 0 == testCount);

            if (doInstantiate) {
                proxy->priv().doLazyInstantiation(ctx->contextPriv().resourceProvider());
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
        GrOpMemoryPool* pool = context->contextPriv().opMemoryPool();

        return pool->allocate<LazyFailedInstantiationTestOp>(proxyProvider,
                                                             testExecuteValue,
                                                             shouldFailInstantiation);
    }

    void visitProxies(const VisitProxyFunc& func) const override {
        func(fLazyProxy.get());
    }

private:
    friend class GrOpMemoryPool; // for ctor

    LazyFailedInstantiationTestOp(GrProxyProvider* proxyProvider, int* testExecuteValue,
                                  bool shouldFailInstantiation)
            : INHERITED(ClassID())
            , fTestExecuteValue(testExecuteValue) {
        GrSurfaceDesc desc;
        desc.fWidth = kSize;
        desc.fHeight = kSize;
        desc.fConfig = kRGBA_8888_GrPixelConfig;

        fLazyProxy = proxyProvider->createLazyProxy(
                [testExecuteValue, shouldFailInstantiation, desc](GrResourceProvider* rp) {
                    if (!rp) {
                        return sk_sp<GrTexture>();
                    }
                    if (shouldFailInstantiation) {
                        *testExecuteValue = 1;
                        return sk_sp<GrTexture>();
                    }
                    return rp->createTexture(desc, SkBudgeted::kNo);
                },
                desc, kTopLeft_GrSurfaceOrigin, GrMipMapped::kNo, SkBackingFit::kExact,
                SkBudgeted::kNo);

        SkASSERT(fLazyProxy.get());

        this->setBounds(SkRect::MakeIWH(kSize, kSize),
                        HasAABloat::kNo, IsZeroArea::kNo);
    }

    const char* name() const override { return "LazyFailedInstantiationTestOp"; }
    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }
    RequiresDstTexture finalize(const GrCaps&, const GrAppliedClip*) override {
        return RequiresDstTexture::kNo;
    }
    bool onCombineIfPossible(GrOp* other, const GrCaps& caps) override { return false; }
    void onPrepare(GrOpFlushState*) override {}
    void onExecute(GrOpFlushState* state) override {
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
    GrResourceProvider* resourceProvider = ctx->contextPriv().resourceProvider();
    GrProxyProvider* proxyProvider = ctx->contextPriv().proxyProvider();
    for (bool failInstantiation : {false, true}) {
        sk_sp<GrRenderTargetContext> rtc = ctx->contextPriv().makeDeferredRenderTargetContext(
                                                     SkBackingFit::kExact, 100, 100,
                                                     kRGBA_8888_GrPixelConfig, nullptr);
        REPORTER_ASSERT(reporter, rtc);

        rtc->clear(nullptr, 0xbaaaaaad, GrRenderTargetContext::CanClearFullscreen::kYes);

        int executeTestValue = 0;
        rtc->priv().testingOnly_addDrawOp(LazyFailedInstantiationTestOp::Make(
                ctx.get(), proxyProvider, &executeTestValue, failInstantiation));
        ctx->flush();

        if (failInstantiation) {
            if (resourceProvider->explicitlyAllocateGPUResources()) {
                REPORTER_ASSERT(reporter, 1 == executeTestValue);
            } else {
                // When we disable explicit gpu resource allocation we don't throw away ops that
                // have uninstantiated proxies.
                REPORTER_ASSERT(reporter, 2 == executeTestValue);
            }
        } else {
            REPORTER_ASSERT(reporter, 2 == executeTestValue);
        }
    }
}

class LazyUninstantiateTestOp : public GrDrawOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrDrawOp> Make(GrContext* context, sk_sp<GrTextureProxy> proxy) {
        GrOpMemoryPool* pool = context->contextPriv().opMemoryPool();

        return pool->allocate<LazyUninstantiateTestOp>(std::move(proxy));
    }

    void visitProxies(const VisitProxyFunc& func) const override {
        func(fLazyProxy.get());
    }

private:
    friend class GrOpMemoryPool; // for ctor

    LazyUninstantiateTestOp(sk_sp<GrTextureProxy> proxy)
            : INHERITED(ClassID())
            , fLazyProxy(std::move(proxy)) {
        this->setBounds(SkRect::MakeIWH(kSize, kSize), HasAABloat::kNo, IsZeroArea::kNo);
    }

    const char* name() const override { return "LazyUninstantiateTestOp"; }
    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }
    RequiresDstTexture finalize(const GrCaps&, const GrAppliedClip*) override {
        return RequiresDstTexture::kNo;
    }
    bool onCombineIfPossible(GrOp* other, const GrCaps& caps) override { return false; }
    void onPrepare(GrOpFlushState*) override {}
    void onExecute(GrOpFlushState* state) override {}

    sk_sp<GrSurfaceProxy> fLazyProxy;

    typedef GrDrawOp INHERITED;
};

static void UninstantiateReleaseProc(void* releaseValue) {
    (*static_cast<int*>(releaseValue))++;
}

// Test that lazy proxies with the Uninstantiate LazyCallbackType are uninstantiated and released as
// expected.
DEF_GPUTEST(LazyProxyUninstantiateTest, reporter, /* options */) {
    GrMockOptions mockOptions;
    sk_sp<GrContext> ctx = GrContext::MakeMock(&mockOptions, GrContextOptions());
    GrProxyProvider* proxyProvider = ctx->contextPriv().proxyProvider();
    GrGpu* gpu = ctx->contextPriv().getGpu();

    using LazyType = GrSurfaceProxy::LazyInstantiationType;
    for (auto lazyType : {LazyType::kSingleUse, LazyType::kMultipleUse, LazyType::kUninstantiate}) {
        sk_sp<GrRenderTargetContext> rtc = ctx->contextPriv().makeDeferredRenderTargetContext(
                SkBackingFit::kExact, 100, 100,
                kRGBA_8888_GrPixelConfig, nullptr);
        REPORTER_ASSERT(reporter, rtc);

        rtc->clear(nullptr, 0xbaaaaaad, GrRenderTargetContext::CanClearFullscreen::kYes);

        int instantiateTestValue = 0;
        int releaseTestValue = 0;
        int* instantiatePtr = &instantiateTestValue;
        int* releasePtr = &releaseTestValue;
        GrSurfaceDesc desc;
        desc.fWidth = kSize;
        desc.fHeight = kSize;
        desc.fConfig = kRGBA_8888_GrPixelConfig;

        GrBackendTexture backendTex = gpu->createTestingOnlyBackendTexture(
                nullptr, kSize, kSize, kRGBA_8888_GrPixelConfig, false, GrMipMapped::kNo);

        sk_sp<GrTextureProxy> lazyProxy = proxyProvider->createLazyProxy(
                [instantiatePtr, releasePtr, backendTex](GrResourceProvider* rp) {
                    if (!rp) {
                        return sk_sp<GrTexture>();
                    }

                    sk_sp<GrTexture> texture = rp->wrapBackendTexture(backendTex);
                    if (!texture) {
                        return sk_sp<GrTexture>();
                    }
                    (*instantiatePtr)++;
                    texture->setRelease(UninstantiateReleaseProc, releasePtr);
                    return texture;
                },
                desc, kTopLeft_GrSurfaceOrigin, GrMipMapped::kNo, GrInternalSurfaceFlags::kNone,
                SkBackingFit::kExact, SkBudgeted::kNo, lazyType);

        REPORTER_ASSERT(reporter, lazyProxy.get());

        rtc->priv().testingOnly_addDrawOp(LazyUninstantiateTestOp::Make(ctx.get(), lazyProxy));

        ctx->flush();

        REPORTER_ASSERT(reporter, 1 == instantiateTestValue);
        if (LazyType::kUninstantiate == lazyType) {
            REPORTER_ASSERT(reporter, 1 == releaseTestValue);
        } else {
            REPORTER_ASSERT(reporter, 0 == releaseTestValue);
        }

        // This should cause the uninstantiate proxies to be instantiated again but have no effect
        // on the others
        rtc->priv().testingOnly_addDrawOp(LazyUninstantiateTestOp::Make(ctx.get(), lazyProxy));
        // Add a second op to make sure we only instantiate once.
        rtc->priv().testingOnly_addDrawOp(LazyUninstantiateTestOp::Make(ctx.get(), lazyProxy));
        ctx->flush();

        if (LazyType::kUninstantiate == lazyType) {
            REPORTER_ASSERT(reporter, 2 == instantiateTestValue);
            REPORTER_ASSERT(reporter, 2 == releaseTestValue);
        } else {
            REPORTER_ASSERT(reporter, 1 == instantiateTestValue);
            REPORTER_ASSERT(reporter, 0 == releaseTestValue);
        }

        lazyProxy.reset();
        if (LazyType::kUninstantiate == lazyType) {
            REPORTER_ASSERT(reporter, 2 == releaseTestValue);
        } else {
            REPORTER_ASSERT(reporter, 1 == releaseTestValue);
        }

        gpu->deleteTestingOnlyBackendTexture(backendTex);
    }
}
