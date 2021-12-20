/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/gpu/mock/GrMockTypes.h"
#include "src/core/SkRectPriv.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOnFlushResourceProvider.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrSurfaceProxy.h"
#include "src/gpu/GrSurfaceProxyPriv.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/GrTextureProxyPriv.h"
#include "src/gpu/effects/GrTextureEffect.h"
#include "src/gpu/mock/GrMockGpu.h"
#include "src/gpu/ops/GrDrawOp.h"
#include "src/gpu/v1/SurfaceDrawContext_v1.h"

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

    void preFlush(GrOnFlushResourceProvider*, SkSpan<const uint32_t>) override {
        REPORTER_ASSERT(fReporter, !fHasOpTexture);
        REPORTER_ASSERT(fReporter, !fHasClipTexture);
    }

    void postFlush(GrDeferredUploadToken, SkSpan<const uint32_t>) override {
        REPORTER_ASSERT(fReporter, fHasOpTexture);
        REPORTER_ASSERT(fReporter, fHasClipTexture);
    }

    class Op final : public GrDrawOp {
    public:
        DEFINE_OP_CLASS_ID

        static GrOp::Owner Make(GrRecordingContext* context,
                                GrProxyProvider* proxyProvider,
                                LazyProxyTest* test,
                                bool nullTexture) {
            return GrOp::Make<Op>(context, context, proxyProvider, test, nullTexture);
        }

        void visitProxies(const GrVisitProxyFunc& func) const override {
            func(fProxy.get(), GrMipmapped::kNo);
        }

        void onExecute(GrOpFlushState*, const SkRect& chainBounds) override {
            REPORTER_ASSERT(fTest->fReporter, fTest->fHasOpTexture);
            REPORTER_ASSERT(fTest->fReporter, fTest->fHasClipTexture);
        }

    private:
        friend class GrOp; // for ctor

        Op(GrRecordingContext* ctx, GrProxyProvider* proxyProvider,
           LazyProxyTest* test, bool nullTexture)
                    : GrDrawOp(ClassID()), fTest(test) {
            const GrBackendFormat format =
                ctx->priv().caps()->getDefaultBackendFormat(GrColorType::kBGR_565,
                                                            GrRenderable::kNo);
            fProxy = GrProxyProvider::MakeFullyLazyProxy(
                    [this, nullTexture](GrResourceProvider* rp,
                                        const GrSurfaceProxy::LazySurfaceDesc& desc)
                            -> GrSurfaceProxy::LazyCallbackResult {
                        REPORTER_ASSERT(fTest->fReporter, !fTest->fHasOpTexture);
                        fTest->fHasOpTexture = true;
                        if (nullTexture) {
                            return {};
                        } else {
                            static constexpr SkISize kDimensions = {1234, 567};
                            sk_sp<GrTexture> texture = rp->createTexture(
                                    kDimensions,
                                    desc.fFormat,
                                    desc.fTextureType,
                                    desc.fRenderable,
                                    desc.fSampleCnt,
                                    desc.fMipmapped,
                                    desc.fBudgeted,
                                    desc.fProtected);
                            REPORTER_ASSERT(fTest->fReporter, texture);
                            return texture;
                        }
                    },
                    format, GrRenderable::kNo, 1, GrProtected::kNo, *proxyProvider->caps(),
                    GrSurfaceProxy::UseAllocator::kYes);

            this->setBounds(SkRectPriv::MakeLargest(), GrOp::HasAABloat::kNo,
                            GrOp::IsHairline::kNo);
        }

        const char* name() const override { return "LazyProxyTest::Op"; }
        FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }
        GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip* clip,
                                          GrClampType) override {
            return GrProcessorSet::EmptySetAnalysis();
        }
        void onPrePrepare(GrRecordingContext*,
                          const GrSurfaceProxyView& writeView,
                          GrAppliedClip*,
                          const GrDstProxyView&,
                          GrXferBarrierFlags renderPassXferBarriers,
                          GrLoadOp colorLoadOp) override {}

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
            static const GrColorType kColorType = GrColorType::kAlpha_F16;
            static const GrSurfaceOrigin kOrigin = kBottomLeft_GrSurfaceOrigin;
            const GrBackendFormat format =
                ctx->priv().caps()->getDefaultBackendFormat(kColorType, GrRenderable::kYes);
            GrSwizzle readSwizzle = ctx->priv().caps()->getReadSwizzle(format, kColorType);
            fLazyProxy = GrProxyProvider::MakeFullyLazyProxy(
                    [this](GrResourceProvider* rp, const GrSurfaceProxy::LazySurfaceDesc&)
                            -> GrSurfaceProxy::LazyCallbackResult {
                        REPORTER_ASSERT(fTest->fReporter, !fTest->fHasClipTexture);
                        fTest->fHasClipTexture = true;
                        fAtlas->instantiate(rp);
                        return sk_ref_sp(fAtlas->peekTexture());
                    },
                    format, GrRenderable::kYes, 1, GrProtected::kNo, *proxyProvider->caps(),
                    GrSurfaceProxy::UseAllocator::kYes);
            auto atlasEffect = GrTextureEffect::Make({fLazyProxy, kOrigin, readSwizzle},
                                                     kPremul_SkAlphaType);
            this->registerChild(std::move(atlasEffect));
        }

    private:
        const char* name() const override { return "LazyProxyTest::ClipFP"; }
        std::unique_ptr<GrFragmentProcessor> clone() const override {
            return std::make_unique<ClipFP>(fContext, fProxyProvider, fTest, fAtlas);
        }
        std::unique_ptr<ProgramImpl> onMakeProgramImpl() const override {
            return nullptr;
        }
        void onAddToKey(const GrShaderCaps&, skgpu::KeyBuilder*) const override {}
        bool onIsEqual(const GrFragmentProcessor&) const override { return false; }

        GrRecordingContext* const fContext;
        GrProxyProvider* const fProxyProvider;
        LazyProxyTest* const fTest;
        GrTextureProxy* const fAtlas;
        sk_sp<GrTextureProxy> fLazyProxy;
    };


    class Clip : public GrClip {
    public:
        Clip(LazyProxyTest* test, GrTextureProxy* atlas)
                : fTest(test)
                , fAtlas(atlas) {}

    private:
        SkIRect getConservativeBounds() const final {
            return SkIRect::MakeSize(fAtlas->dimensions());
        }
        Effect apply(GrRecordingContext* rContext,
                     skgpu::v1::SurfaceDrawContext*,
                     GrDrawOp*,
                     GrAAType,
                     GrAppliedClip* out,
                     SkRect* bounds) const override {
            GrProxyProvider* proxyProvider = rContext->priv().proxyProvider();
            out->addCoverageFP(std::make_unique<ClipFP>(rContext, proxyProvider, fTest, fAtlas));
            return Effect::kClipped;
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
    sk_sp<GrDirectContext> ctx = GrDirectContext::MakeMock(&mockOptions, GrContextOptions());
    GrProxyProvider* proxyProvider = ctx->priv().proxyProvider();
    for (bool nullTexture : {false, true}) {
        LazyProxyTest test(reporter);
        ctx->priv().addOnFlushCallbackObject(&test);
        auto sdc = skgpu::v1::SurfaceDrawContext::Make(ctx.get(), GrColorType::kRGBA_8888, nullptr,
                                                       SkBackingFit::kExact, {100, 100},
                                                       SkSurfaceProps());
        REPORTER_ASSERT(reporter, sdc);
        auto mockAtlas = skgpu::v1::SurfaceDrawContext::Make(ctx.get(), GrColorType::kAlpha_F16,
                                                             nullptr, SkBackingFit::kExact,
                                                             {10, 10}, SkSurfaceProps());
        REPORTER_ASSERT(reporter, mockAtlas);
        LazyProxyTest::Clip clip(&test, mockAtlas->asTextureProxy());
        sdc->addDrawOp(&clip,
                       LazyProxyTest::Op::Make(ctx.get(), proxyProvider, &test, nullTexture));
        ctx->priv().testingOnly_flushAndRemoveOnFlushCallbackObject(&test);
    }
}

static const int kSize = 16;

DEF_GPUTEST(LazyProxyReleaseTest, reporter, /* options */) {
    GrMockOptions mockOptions;
    sk_sp<GrDirectContext> ctx = GrDirectContext::MakeMock(&mockOptions, GrContextOptions());
    auto proxyProvider = ctx->priv().proxyProvider();
    const GrCaps* caps = ctx->priv().caps();

    GrBackendFormat format = caps->getDefaultBackendFormat(GrColorType::kRGBA_8888,
                                                           GrRenderable::kNo);

    auto tex = ctx->priv().resourceProvider()->createTexture({kSize, kSize},
                                                             format,
                                                             GrTextureType::k2D,
                                                             GrRenderable::kNo,
                                                             1,
                                                             GrMipmapped::kNo,
                                                             SkBudgeted::kNo,
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
                    fValue = std::exchange(that.fValue, nullptr);
                    return *this;
                }
                TestCallback& operator=(const TestCallback& that) = delete;

                LazyInstantiationResult operator()(GrResourceProvider*,
                                                   const GrSurfaceProxy::LazySurfaceDesc&) const {
                    *fValue = 1;
                    return {fTexture, fReleaseCallback};
                }

            private:
                int* fValue = nullptr;
                bool fReleaseCallback;
                sk_sp<GrTexture> fTexture;
            };
            sk_sp<GrTextureProxy> proxy = proxyProvider->createLazyProxy(
                    TestCallback(&testCount, releaseCallback, tex), format, {kSize, kSize},
                    GrMipmapped::kNo, GrMipmapStatus::kNotAllocated, GrInternalSurfaceFlags::kNone,
                    SkBackingFit::kExact, SkBudgeted::kNo, GrProtected::kNo,
                    GrSurfaceProxy::UseAllocator::kYes);

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

    static GrOp::Owner Make(GrRecordingContext* rContext,
                            GrProxyProvider* proxyProvider,
                            int* testExecuteValue,
                            bool shouldFailInstantiation) {
        return GrOp::Make<LazyFailedInstantiationTestOp>(rContext,
                                                         rContext->priv().caps(),
                                                         proxyProvider,
                                                         testExecuteValue,
                                                         shouldFailInstantiation);
    }

    void visitProxies(const GrVisitProxyFunc& func) const override {
        func(fLazyProxy.get(), GrMipmapped::kNo);
    }

private:
    friend class GrOp; // for ctor

    LazyFailedInstantiationTestOp(const GrCaps* caps, GrProxyProvider* proxyProvider,
                                  int* testExecuteValue, bool shouldFailInstantiation)
            : INHERITED(ClassID())
            , fTestExecuteValue(testExecuteValue) {
        SkISize dims = {kSize, kSize};
        GrBackendFormat format = caps->getDefaultBackendFormat(GrColorType::kRGBA_8888,
                                                               GrRenderable::kNo);

        fLazyProxy = proxyProvider->createLazyProxy(
                [testExecuteValue, shouldFailInstantiation](
                        GrResourceProvider* rp, const GrSurfaceProxy::LazySurfaceDesc& desc)
                        -> GrSurfaceProxy::LazyCallbackResult {
                    if (shouldFailInstantiation) {
                        *testExecuteValue = 1;
                        return {};
                    }
                    return {rp->createTexture(desc.fDimensions,
                                              desc.fFormat,
                                              desc.fTextureType,
                                              desc.fRenderable,
                                              desc.fSampleCnt,
                                              desc.fMipmapped,
                                              desc.fBudgeted,
                                              desc.fProtected),
                            true, GrSurfaceProxy::LazyInstantiationKeyMode::kUnsynced};
                },
                format, dims, GrMipmapped::kNo, GrMipmapStatus::kNotAllocated,
                GrInternalSurfaceFlags::kNone, SkBackingFit::kExact, SkBudgeted::kNo,
                GrProtected::kNo, GrSurfaceProxy::UseAllocator::kYes);

        SkASSERT(fLazyProxy.get());

        this->setBounds(SkRect::Make(dims), HasAABloat::kNo, IsHairline::kNo);
    }

    const char* name() const override { return "LazyFailedInstantiationTestOp"; }
    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }
    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*, GrClampType) override {
        return GrProcessorSet::EmptySetAnalysis();
    }
    void onPrePrepare(GrRecordingContext*,
                      const GrSurfaceProxyView& writeView,
                      GrAppliedClip*,
                      const GrDstProxyView&,
                      GrXferBarrierFlags renderPassXferBarriers,
                      GrLoadOp colorLoadOp) override {}
    void onPrepare(GrOpFlushState*) override {}
    void onExecute(GrOpFlushState* state, const SkRect& chainBounds) override {
        *fTestExecuteValue = 2;
    }

    int* fTestExecuteValue;
    sk_sp<GrTextureProxy> fLazyProxy;

    using INHERITED = GrDrawOp;
};

// Test that when a lazy proxy fails to instantiate during flush that we drop the Op that it was
// associated with.
DEF_GPUTEST(LazyProxyFailedInstantiationTest, reporter, /* options */) {
    GrMockOptions mockOptions;
    sk_sp<GrDirectContext> ctx = GrDirectContext::MakeMock(&mockOptions, GrContextOptions());
    GrProxyProvider* proxyProvider = ctx->priv().proxyProvider();
    for (bool failInstantiation : {false, true}) {
        auto sdc = skgpu::v1::SurfaceDrawContext::Make(ctx.get(), GrColorType::kRGBA_8888, nullptr,
                                                       SkBackingFit::kExact, {100, 100},
                                                       SkSurfaceProps());
        REPORTER_ASSERT(reporter, sdc);

        sdc->clear(SkPMColor4f::FromBytes_RGBA(0xbaaaaaad));

        int executeTestValue = 0;
        sdc->addDrawOp(LazyFailedInstantiationTestOp::Make(ctx.get(), proxyProvider,
                                                           &executeTestValue, failInstantiation));
        ctx->flushAndSubmit();

        if (failInstantiation) {
            REPORTER_ASSERT(reporter, 1 == executeTestValue);
        } else {
            REPORTER_ASSERT(reporter, 2 == executeTestValue);
        }
    }
}
