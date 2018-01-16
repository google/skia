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

        Op(LazyProxyTest* test, bool nullTexture) : GrDrawOp(ClassID()), fTest(test) {
            fProxy = GrSurfaceProxy::MakeFullyLazy([this, nullTexture](GrResourceProvider* rp,
                                                                       GrSurfaceOrigin* origin) {
                REPORTER_ASSERT(fTest->fReporter, !fTest->fHasOpTexture);
                fTest->fHasOpTexture = true;
                *origin = kTopLeft_GrSurfaceOrigin;
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
            }, GrSurfaceProxy::Renderable::kNo, kRGB_565_GrPixelConfig);
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
        ClipFP(LazyProxyTest* test, GrTextureProxy* atlas)
                : GrFragmentProcessor(kTestFP_ClassID, kNone_OptimizationFlags)
                , fTest(test)
                , fAtlas(atlas) {
            fLazyProxy = GrSurfaceProxy::MakeFullyLazy([this](GrResourceProvider* rp,
                                                              GrSurfaceOrigin* origin) {
                REPORTER_ASSERT(fTest->fReporter, !fTest->fHasClipTexture);
                fTest->fHasClipTexture = true;
                *origin = kBottomLeft_GrSurfaceOrigin;
                fAtlas->instantiate(rp);
                return sk_ref_sp(fAtlas->priv().peekTexture());
            }, GrSurfaceProxy::Renderable::kYes, kAlpha_half_GrPixelConfig);
            fAccess.reset(fLazyProxy, GrSamplerState::Filter::kNearest,
                          GrSamplerState::WrapMode::kClamp, kFragment_GrShaderFlag);
            this->addTextureSampler(&fAccess);
        }

    private:
        const char* name() const override { return "LazyProxyTest::ClipFP"; }
        std::unique_ptr<GrFragmentProcessor> clone() const override {
            return skstd::make_unique<ClipFP>(fTest, fAtlas);
        }
        GrGLSLFragmentProcessor* onCreateGLSLInstance() const override { return nullptr; }
        void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {}
        bool onIsEqual(const GrFragmentProcessor&) const override { return false; }

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
        bool apply(GrContext*, GrRenderTargetContext*, bool, bool, GrAppliedClip* out,
                   SkRect* bounds) const override {
            out->addCoverageFP(skstd::make_unique<ClipFP>(fTest, fAtlas));
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
    mockOptions.fConfigOptions[kAlpha_half_GrPixelConfig].fRenderable[0] = true;
    mockOptions.fConfigOptions[kAlpha_half_GrPixelConfig].fTexturable = true;
    sk_sp<GrContext> ctx = GrContext::MakeMock(&mockOptions, GrContextOptions());
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
                                         skstd::make_unique<LazyProxyTest::Op>(&test, nullTexture));
        ctx->contextPriv().testingOnly_flushAndRemoveOnFlushCallbackObject(&test);
    }
}

DEF_GPUTEST(LazyProxyReleaseTest, reporter, /* options */) {
    GrMockOptions mockOptions;
    sk_sp<GrContext> ctx = GrContext::MakeMock(&mockOptions, GrContextOptions());

    GrSurfaceDesc desc;
    desc.fWidth = 16;
    desc.fHeight = 16;
    desc.fConfig = kRGBA_8888_GrPixelConfig;

    for (bool doInstantiate : {true, false}) {
        int testCount = 0;
        int* testCountPtr = &testCount;
        sk_sp<GrTextureProxy> proxy = GrSurfaceProxy::MakeLazy(
                [testCountPtr](GrResourceProvider* resourceProvider, GrSurfaceOrigin* outOrigin) {
                    if (!resourceProvider) {
                        *testCountPtr = -1;
                        return sk_sp<GrTexture>();
                    }
                    *testCountPtr = 1;
                    return sk_sp<GrTexture>();
                }, desc, GrMipMapped::kNo, SkBackingFit::kExact, SkBudgeted::kNo);

        REPORTER_ASSERT(reporter, 0 == testCount);

        if (doInstantiate) {
            proxy->priv().doLazyInstantiation(ctx->contextPriv().resourceProvider());
            REPORTER_ASSERT(reporter, 1 == testCount);
            proxy.reset();
            REPORTER_ASSERT(reporter, 1 == testCount);
        } else {
            proxy.reset();
            REPORTER_ASSERT(reporter, -1 == testCount);
        }
    }
}

#endif
