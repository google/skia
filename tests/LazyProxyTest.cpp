/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#if SK_SUPPORT_GPU

#include "GrClip.h"
#include "GrContextPriv.h"
#include "GrProxyProvider.h"
#include "GrOnFlushResourceProvider.h"
#include "GrRenderTargetContext.h"
#include "GrRenderTargetContextPriv.h"
#include "GrSurfaceProxy.h"
#include "GrSurfaceProxyPriv.h"
#include "GrTexture.h"
#include "GrTextureProxy.h"
#include "GrTextureProxyPriv.h"
#include "SkMakeUnique.h"
#include "SkRectPriv.h"
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

        Op(GrProxyProvider* proxyProvider, LazyProxyTest* test, bool nullTexture)
                    : GrDrawOp(ClassID()), fTest(test) {
            fProxy = proxyProvider->createFullyLazyProxy([this, nullTexture](
                                        GrResourceProvider* rp) {
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
                    desc.fOrigin = kTopLeft_GrSurfaceOrigin;
                    desc.fConfig = kRGB_565_GrPixelConfig;
                    sk_sp<GrTexture> texture = rp->createTexture(desc, SkBudgeted::kYes);
                    REPORTER_ASSERT(fTest->fReporter, texture);
                    return texture;
                }
            }, GrProxyProvider::Renderable::kNo, kTopLeft_GrSurfaceOrigin, kRGB_565_GrPixelConfig);
            this->setBounds(SkRectPriv::MakeLargest(), GrOp::HasAABloat::kNo, GrOp::IsZeroArea::kNo);
        }

        void visitProxies(const VisitProxyFunc& func) const override {
            func(fProxy.get());
        }

        void onExecute(GrOpFlushState*) override {
            REPORTER_ASSERT(fTest->fReporter, fTest->fHasOpTexture);
            REPORTER_ASSERT(fTest->fReporter, fTest->fHasClipTexture);
        }

    private:
        const char* name() const override { return "LazyProxyTest::Op"; }
        FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }
        RequiresDstTexture finalize(const GrCaps&, const GrAppliedClip*,
                                    GrPixelConfigIsClamped) override {
            return RequiresDstTexture::kNo;
        }
        void wasRecorded(GrRenderTargetOpList*) override {}
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
            fLazyProxy = proxyProvider->createFullyLazyProxy(
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
                                kAlpha_half_GrPixelConfig);
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
        sk_sp<GrRenderTargetContext> rtc =
                ctx->makeDeferredRenderTargetContext(SkBackingFit::kExact, 100, 100,
                                                     kRGBA_8888_GrPixelConfig, nullptr);
        REPORTER_ASSERT(reporter, rtc);
        sk_sp<GrRenderTargetContext> mockAtlas =
                ctx->makeDeferredRenderTargetContext(SkBackingFit::kExact, 10, 10,
                                                     kAlpha_half_GrPixelConfig, nullptr);
        REPORTER_ASSERT(reporter, mockAtlas);
        rtc->priv().testingOnly_addDrawOp(LazyProxyTest::Clip(&test, mockAtlas->asTextureProxy()),
                        skstd::make_unique<LazyProxyTest::Op>(proxyProvider, &test, nullTexture));
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
                              LazyInstantiationType::kMultipleUse}) {
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
                    }, desc, GrMipMapped::kNo, SkBackingFit::kExact, SkBudgeted::kNo);

            proxy->priv().testingOnly_setLazyInstantiationType(lazyType);

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

    LazyFailedInstantiationTestOp(GrProxyProvider* proxyProvider, int* testExecuteValue,
                                  bool shouldFailInstantiation)
            : INHERITED(ClassID())
            , fTestExecuteValue(testExecuteValue) {
        GrSurfaceDesc desc;
        desc.fWidth = kSize;
        desc.fHeight = kSize;
        desc.fConfig = kRGBA_8888_GrPixelConfig;

        fLazyProxy = proxyProvider->createLazyProxy(
                [testExecuteValue, shouldFailInstantiation, desc] (GrResourceProvider* rp) {
                    if (!rp) {
                        return sk_sp<GrTexture>();
                    }
                    if (shouldFailInstantiation) {
                        *testExecuteValue = 1;
                        return sk_sp<GrTexture>();
                    }
                    return rp->createTexture(desc, SkBudgeted::kNo);
                }, desc, GrMipMapped::kNo, SkBackingFit::kExact, SkBudgeted::kNo);

        this->setBounds(SkRect::MakeIWH(kSize, kSize),
                        HasAABloat::kNo, IsZeroArea::kNo);
    }

    void visitProxies(const VisitProxyFunc& func) const override {
        func(fLazyProxy.get());
    }

private:
    const char* name() const override { return "LazyFailedInstantiationTestOp"; }
    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }
    RequiresDstTexture finalize(const GrCaps&, const GrAppliedClip*,
                                GrPixelConfigIsClamped) override {
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
        sk_sp<GrRenderTargetContext> rtc =
                ctx->makeDeferredRenderTargetContext(SkBackingFit::kExact, 100, 100,
                                                     kRGBA_8888_GrPixelConfig, nullptr);
        REPORTER_ASSERT(reporter, rtc);

        rtc->clear(nullptr, 0xbaaaaaad, GrRenderTargetContext::CanClearFullscreen::kYes);

        int executeTestValue = 0;
        rtc->priv().testingOnly_addDrawOp(
                skstd::make_unique<LazyFailedInstantiationTestOp>(proxyProvider, &executeTestValue,
                                                                  failInstantiation));
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

#endif
