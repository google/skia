/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/mock/GrMockTypes.h"
#include "include/private/SkColorData.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkRectPriv.h"
#include "src/gpu/AtlasTypes.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/ganesh/GrAppliedClip.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrClip.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrOnFlushResourceProvider.h"
#include "src/gpu/ganesh/GrProcessorSet.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrResourceProvider.h"
#include "src/gpu/ganesh/GrSurface.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyPriv.h"
#include "src/gpu/ganesh/GrTexture.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "src/gpu/ganesh/effects/GrTextureEffect.h"
#include "src/gpu/ganesh/ops/GrDrawOp.h"
#include "src/gpu/ganesh/ops/GrOp.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"

#include <functional>
#include <initializer_list>
#include <memory>
#include <utility>

class GrDstProxyView;
class GrOpFlushState;
class GrSurfaceProxyView;
enum class GrXferBarrierFlags;
namespace skgpu { class KeyBuilder; }
struct GrShaderCaps;

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

    bool preFlush(GrOnFlushResourceProvider* onFlushRP) override {
#if defined(GR_TEST_UTILS)
        if (onFlushRP->failFlushTimeCallbacks()) {
            return false;
        }
#endif

        REPORTER_ASSERT(fReporter, !fHasOpTexture);
        REPORTER_ASSERT(fReporter, !fHasClipTexture);
        return true;
    }

    void postFlush(skgpu::AtlasToken) override {
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
            func(fProxy.get(), skgpu::Mipmapped::kNo);
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
                            sk_sp<GrTexture> texture = rp->createTexture(kDimensions,
                                                                         desc.fFormat,
                                                                         desc.fTextureType,
                                                                         desc.fRenderable,
                                                                         desc.fSampleCnt,
                                                                         desc.fMipmapped,
                                                                         desc.fBudgeted,
                                                                         desc.fProtected,
                                                                         /*label=*/{});
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
            skgpu::Swizzle readSwizzle = ctx->priv().caps()->getReadSwizzle(format, kColorType);
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
                     skgpu::ganesh::SurfaceDrawContext*,
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

DEF_GANESH_TEST(LazyProxyTest, reporter, /* options */, CtsEnforcement::kApiLevel_T) {
    GrMockOptions mockOptions;
    mockOptions.fConfigOptions[(int)GrColorType::kAlpha_F16].fRenderability =
            GrMockOptions::ConfigOptions::Renderability::kNonMSAA;
    mockOptions.fConfigOptions[(int)GrColorType::kAlpha_F16].fTexturable = true;
    sk_sp<GrDirectContext> ctx = GrDirectContext::MakeMock(&mockOptions, GrContextOptions());
    GrProxyProvider* proxyProvider = ctx->priv().proxyProvider();
    for (bool nullTexture : {false, true}) {
        LazyProxyTest test(reporter);
        ctx->priv().addOnFlushCallbackObject(&test);
        auto sdc = skgpu::ganesh::SurfaceDrawContext::Make(ctx.get(),
                                                           GrColorType::kRGBA_8888,
                                                           nullptr,
                                                           SkBackingFit::kExact,
                                                           {100, 100},
                                                           SkSurfaceProps(),
                                                           /*label=*/{});
        REPORTER_ASSERT(reporter, sdc);
        auto mockAtlas = skgpu::ganesh::SurfaceDrawContext::Make(ctx.get(),
                                                                 GrColorType::kAlpha_F16,
                                                                 nullptr,
                                                                 SkBackingFit::kExact,
                                                                 {10, 10},
                                                                 SkSurfaceProps(),
                                                                 /*label=*/{});
        REPORTER_ASSERT(reporter, mockAtlas);
        LazyProxyTest::Clip clip(&test, mockAtlas->asTextureProxy());
        sdc->addDrawOp(&clip,
                       LazyProxyTest::Op::Make(ctx.get(), proxyProvider, &test, nullTexture));
        ctx->priv().testingOnly_flushAndRemoveOnFlushCallbackObject(&test);
    }
}

static const int kSize = 16;

DEF_GANESH_TEST(LazyProxyReleaseTest, reporter, /* options */, CtsEnforcement::kApiLevel_T) {
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
                                                             skgpu::Mipmapped::kNo,
                                                             skgpu::Budgeted::kNo,
                                                             GrProtected::kNo,
                                                             /*label=*/{});
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
            sk_sp<GrTextureProxy> proxy =
                    proxyProvider->createLazyProxy(TestCallback(&testCount, releaseCallback, tex),
                                                   format,
                                                   {kSize, kSize},
                                                   skgpu::Mipmapped::kNo,
                                                   GrMipmapStatus::kNotAllocated,
                                                   GrInternalSurfaceFlags::kNone,
                                                   SkBackingFit::kExact,
                                                   skgpu::Budgeted::kNo,
                                                   GrProtected::kNo,
                                                   GrSurfaceProxy::UseAllocator::kYes,
                                                   /*label=*/{});

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
        func(fLazyProxy.get(), skgpu::Mipmapped::kNo);
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
                                              desc.fProtected,
                                              /*label=*/{}),
                            true, GrSurfaceProxy::LazyInstantiationKeyMode::kUnsynced};
                },
                format,
                dims,
                skgpu::Mipmapped::kNo,
                GrMipmapStatus::kNotAllocated,
                GrInternalSurfaceFlags::kNone,
                SkBackingFit::kExact,
                skgpu::Budgeted::kNo,
                GrProtected::kNo,
                GrSurfaceProxy::UseAllocator::kYes,
                /*label=*/{});

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
DEF_GANESH_TEST(LazyProxyFailedInstantiationTest,
                reporter,
                /* options */,
                CtsEnforcement::kApiLevel_T) {
    GrMockOptions mockOptions;
    sk_sp<GrDirectContext> ctx = GrDirectContext::MakeMock(&mockOptions, GrContextOptions());
    GrProxyProvider* proxyProvider = ctx->priv().proxyProvider();
    for (bool failInstantiation : {false, true}) {
        auto sdc = skgpu::ganesh::SurfaceDrawContext::Make(ctx.get(),
                                                           GrColorType::kRGBA_8888,
                                                           nullptr,
                                                           SkBackingFit::kExact,
                                                           {100, 100},
                                                           SkSurfaceProps(),
                                                           /*label=*/{});
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
