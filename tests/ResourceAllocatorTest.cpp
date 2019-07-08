/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#include "tests/Test.h"

#include "include/gpu/GrTexture.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrDeinstantiateProxyTracker.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrResourceAllocator.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrSurfaceProxyPriv.h"
#include "src/gpu/GrTextureProxy.h"

#include "include/core/SkSurface.h"

struct ProxyParams {
    int             fSize;
    bool            fIsRT;
    SkColorType     fColorType;
    SkBackingFit    fFit;
    int             fSampleCnt;
    GrSurfaceOrigin fOrigin;
    SkBudgeted      fBudgeted;
    // TODO: do we care about mipmapping
};

static sk_sp<GrSurfaceProxy> make_deferred(GrProxyProvider* proxyProvider, const GrCaps* caps,
                                           const ProxyParams& p) {
    GrColorType grCT = SkColorTypeToGrColorType(p.fColorType);
    GrPixelConfig config = GrColorTypeToPixelConfig(grCT, GrSRGBEncoded::kNo);

    GrSurfaceDesc desc;
    desc.fFlags = p.fIsRT ? kRenderTarget_GrSurfaceFlag : kNone_GrSurfaceFlags;
    desc.fWidth  = p.fSize;
    desc.fHeight = p.fSize;
    desc.fConfig = config;
    desc.fSampleCnt = p.fSampleCnt;

    const GrBackendFormat format = caps->getBackendFormatFromColorType(grCT);

    return proxyProvider->createProxy(format, desc, p.fOrigin, p.fFit, p.fBudgeted);
}

static sk_sp<GrSurfaceProxy> make_backend(GrContext* context, const ProxyParams& p,
                                          GrBackendTexture* backendTex) {
    GrProxyProvider* proxyProvider = context->priv().proxyProvider();

    *backendTex = context->createBackendTexture(p.fSize, p.fSize, p.fColorType,
                                                SkColors::kTransparent,
                                                GrMipMapped::kNo, GrRenderable::kNo,
                                                GrProtected::kNo);
    if (!backendTex->isValid()) {
        return nullptr;
    }

    return proxyProvider->wrapBackendTexture(*backendTex, p.fOrigin, kBorrow_GrWrapOwnership,
                                             GrWrapCacheable::kNo, kRead_GrIOType);
}

static void cleanup_backend(GrContext* context, const GrBackendTexture& backendTex) {
    context->deleteBackendTexture(backendTex);
}

// Basic test that two proxies with overlapping intervals and compatible descriptors are
// assigned different GrSurfaces.
static void overlap_test(skiatest::Reporter* reporter, GrResourceProvider* resourceProvider,
                         sk_sp<GrSurfaceProxy> p1, sk_sp<GrSurfaceProxy> p2,
                         bool expectedResult) {
    GrDeinstantiateProxyTracker deinstantiateTracker;
    GrResourceAllocator alloc(resourceProvider, &deinstantiateTracker SkDEBUGCODE(, 1));

    alloc.addInterval(p1.get(), 0, 4, GrResourceAllocator::ActualUse::kYes);
    alloc.incOps();
    alloc.addInterval(p2.get(), 1, 2, GrResourceAllocator::ActualUse::kYes);
    alloc.incOps();
    alloc.markEndOfOpList(0);

    alloc.determineRecyclability();

    int startIndex, stopIndex;
    GrResourceAllocator::AssignError error;
    alloc.assign(&startIndex, &stopIndex, &error);
    REPORTER_ASSERT(reporter, GrResourceAllocator::AssignError::kNoError == error);

    REPORTER_ASSERT(reporter, p1->peekSurface());
    REPORTER_ASSERT(reporter, p2->peekSurface());
    bool doTheBackingStoresMatch = p1->underlyingUniqueID() == p2->underlyingUniqueID();
    REPORTER_ASSERT(reporter, expectedResult == doTheBackingStoresMatch);
}

// Test various cases when two proxies do not have overlapping intervals.
// This mainly acts as a test of the ResourceAllocator's free pool.
static void non_overlap_test(skiatest::Reporter* reporter, GrResourceProvider* resourceProvider,
                             sk_sp<GrSurfaceProxy> p1, sk_sp<GrSurfaceProxy> p2,
                             bool expectedResult) {
    GrDeinstantiateProxyTracker deinstantiateTracker;
    GrResourceAllocator alloc(resourceProvider, &deinstantiateTracker SkDEBUGCODE(, 1));

    alloc.incOps();
    alloc.incOps();
    alloc.incOps();
    alloc.incOps();
    alloc.incOps();
    alloc.incOps();

    alloc.addInterval(p1.get(), 0, 2, GrResourceAllocator::ActualUse::kYes);
    alloc.addInterval(p2.get(), 3, 5, GrResourceAllocator::ActualUse::kYes);
    alloc.markEndOfOpList(0);

    alloc.determineRecyclability();

    int startIndex, stopIndex;
    GrResourceAllocator::AssignError error;
    alloc.assign(&startIndex, &stopIndex, &error);
    REPORTER_ASSERT(reporter, GrResourceAllocator::AssignError::kNoError == error);

    REPORTER_ASSERT(reporter, p1->peekSurface());
    REPORTER_ASSERT(reporter, p2->peekSurface());
    bool doTheBackingStoresMatch = p1->underlyingUniqueID() == p2->underlyingUniqueID();
    REPORTER_ASSERT(reporter, expectedResult == doTheBackingStoresMatch);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ResourceAllocatorTest, reporter, ctxInfo) {
    const GrCaps* caps = ctxInfo.grContext()->priv().caps();
    GrProxyProvider* proxyProvider = ctxInfo.grContext()->priv().proxyProvider();
    GrResourceProvider* resourceProvider = ctxInfo.grContext()->priv().resourceProvider();

    struct TestCase {
        ProxyParams   fP1;
        ProxyParams   fP2;
        bool          fExpectation;
    };

    constexpr bool kRT = true;
    constexpr bool kNotRT = false;

    constexpr bool kShare = true;
    constexpr bool kDontShare = false;
    // Non-RT GrSurfaces are never recycled on some platforms.
    bool kConditionallyShare = resourceProvider->caps()->reuseScratchTextures();

    const SkColorType kRGBA = kRGBA_8888_SkColorType;
    const SkColorType kBGRA = kBGRA_8888_SkColorType;

    const SkBackingFit kE = SkBackingFit::kExact;
    const SkBackingFit kA = SkBackingFit::kApprox;

    const GrSurfaceOrigin kTL = kTopLeft_GrSurfaceOrigin;
    const GrSurfaceOrigin kBL = kBottomLeft_GrSurfaceOrigin;

    const SkBudgeted kNotB = SkBudgeted::kNo;

    //--------------------------------------------------------------------------------------------
    TestCase gOverlappingTests[] = {
        //----------------------------------------------------------------------------------------
        // Two proxies with overlapping intervals and compatible descriptors should never share
        // RT version
        { { 64,    kRT, kRGBA, kA, 0, kTL, kNotB }, { 64,    kRT, kRGBA, kA, 0, kTL, kNotB }, kDontShare },
        // non-RT version
        { { 64, kNotRT, kRGBA, kA, 0, kTL, kNotB }, { 64, kNotRT, kRGBA, kA, 0, kTL, kNotB }, kDontShare },
    };

    for (auto test : gOverlappingTests) {
        sk_sp<GrSurfaceProxy> p1 = make_deferred(proxyProvider, caps, test.fP1);
        sk_sp<GrSurfaceProxy> p2 = make_deferred(proxyProvider, caps, test.fP2);
        overlap_test(reporter, resourceProvider, std::move(p1), std::move(p2), test.fExpectation);
    }

    int k2 = ctxInfo.grContext()->priv().caps()->getRenderTargetSampleCount(
                                                                    2, kRGBA_8888_GrPixelConfig);
    int k4 = ctxInfo.grContext()->priv().caps()->getRenderTargetSampleCount(
                                                                    4, kRGBA_8888_GrPixelConfig);

    //--------------------------------------------------------------------------------------------
    TestCase gNonOverlappingTests[] = {
        //----------------------------------------------------------------------------------------
        // Two non-overlapping intervals w/ compatible proxies should share
        // both same size & approx
        { { 64,    kRT, kRGBA, kA, 0, kTL, kNotB }, { 64,    kRT, kRGBA, kA, 0, kTL, kNotB }, kShare },
        { { 64, kNotRT, kRGBA, kA, 0, kTL, kNotB }, { 64, kNotRT, kRGBA, kA, 0, kTL, kNotB }, kConditionallyShare },
        // diffs sizes but still approx
        { { 64,    kRT, kRGBA, kA, 0, kTL, kNotB }, { 50,    kRT, kRGBA, kA, 0, kTL, kNotB }, kShare },
        { { 64, kNotRT, kRGBA, kA, 0, kTL, kNotB }, { 50, kNotRT, kRGBA, kA, 0, kTL, kNotB }, kConditionallyShare },
        // sames sizes but exact
        { { 64,    kRT, kRGBA, kE, 0, kTL, kNotB }, { 64,    kRT, kRGBA, kE, 0, kTL, kNotB }, kShare },
        { { 64, kNotRT, kRGBA, kE, 0, kTL, kNotB }, { 64, kNotRT, kRGBA, kE, 0, kTL, kNotB }, kConditionallyShare },
        //----------------------------------------------------------------------------------------
        // Two non-overlapping intervals w/ different exact sizes should not share
        { { 56,    kRT, kRGBA, kE, 0, kTL, kNotB }, { 54,    kRT, kRGBA, kE, 0, kTL, kNotB }, kDontShare },
        // Two non-overlapping intervals w/ _very different_ approx sizes should not share
        { { 255,   kRT, kRGBA, kA, 0, kTL, kNotB }, { 127,   kRT, kRGBA, kA, 0, kTL, kNotB }, kDontShare },
        // Two non-overlapping intervals w/ different MSAA sample counts should not share
        { { 64,    kRT, kRGBA, kA, k2, kTL, kNotB },{ 64,    kRT, kRGBA, kA, k4,kTL, kNotB}, k2 == k4 },
        // Two non-overlapping intervals w/ different configs should not share
        { { 64,    kRT, kRGBA, kA, 0, kTL, kNotB }, { 64,    kRT, kBGRA, kA, 0, kTL, kNotB }, kDontShare },
        // Two non-overlapping intervals w/ different RT classifications should never share
        { { 64,    kRT, kRGBA, kA, 0, kTL, kNotB }, { 64, kNotRT, kRGBA, kA, 0, kTL, kNotB }, kDontShare },
        { { 64, kNotRT, kRGBA, kA, 0, kTL, kNotB }, { 64,    kRT, kRGBA, kA, 0, kTL, kNotB }, kDontShare },
        // Two non-overlapping intervals w/ different origins should share
        { { 64,    kRT, kRGBA, kA, 0, kTL, kNotB }, { 64,    kRT, kRGBA, kA, 0, kBL, kNotB }, kShare },
    };

    for (auto test : gNonOverlappingTests) {
        sk_sp<GrSurfaceProxy> p1 = make_deferred(proxyProvider, caps, test.fP1);
        sk_sp<GrSurfaceProxy> p2 = make_deferred(proxyProvider, caps, test.fP2);

        if (!p1 || !p2) {
            continue; // creation can fail (i.e., for msaa4 on iOS)
        }

        non_overlap_test(reporter, resourceProvider, std::move(p1), std::move(p2),
                         test.fExpectation);
    }

    {
        // Wrapped backend textures should never be reused
        TestCase t[1] = {
            { { 64, kNotRT, kRGBA, kE, 0, kTL, kNotB }, { 64, kNotRT, kRGBA, kE, 0, kTL, kNotB }, kDontShare }
        };

        GrBackendTexture backEndTex;
        sk_sp<GrSurfaceProxy> p1 = make_backend(ctxInfo.grContext(), t[0].fP1, &backEndTex);
        sk_sp<GrSurfaceProxy> p2 = make_deferred(proxyProvider, caps, t[0].fP2);

        non_overlap_test(reporter, resourceProvider, std::move(p1), std::move(p2),
                         t[0].fExpectation);

        cleanup_backend(ctxInfo.grContext(), backEndTex);
    }
}

static void draw(GrContext* context) {
    SkImageInfo ii = SkImageInfo::Make(1024, 1024, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    sk_sp<SkSurface> s = SkSurface::MakeRenderTarget(context, SkBudgeted::kYes,
                                                     ii, 1, kTopLeft_GrSurfaceOrigin, nullptr);

    SkCanvas* c = s->getCanvas();

    c->clear(SK_ColorBLACK);
}


DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ResourceAllocatorStressTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    int maxNum;
    size_t maxBytes;
    context->getResourceCacheLimits(&maxNum, &maxBytes);

    context->setResourceCacheLimits(0, 0); // We'll always be overbudget

    draw(context);
    draw(context);
    draw(context);
    draw(context);
    context->flush();

    context->setResourceCacheLimits(maxNum, maxBytes);
}

sk_sp<GrSurfaceProxy> make_lazy(GrProxyProvider* proxyProvider, const GrCaps* caps,
                                const ProxyParams& p, bool deinstantiate) {
    GrColorType grCT = SkColorTypeToGrColorType(p.fColorType);
    GrPixelConfig config = GrColorTypeToPixelConfig(grCT, GrSRGBEncoded::kNo);

    GrSurfaceDesc desc;
    desc.fFlags = p.fIsRT ? kRenderTarget_GrSurfaceFlag : kNone_GrSurfaceFlags;
    desc.fWidth = p.fSize;
    desc.fHeight = p.fSize;
    desc.fConfig = config;
    desc.fSampleCnt = p.fSampleCnt;

    SkBackingFit fit = p.fFit;
    auto callback = [fit, desc](GrResourceProvider* resourceProvider) {
        sk_sp<GrTexture> texture;
        if (fit == SkBackingFit::kApprox) {
            texture = resourceProvider->createApproxTexture(
                desc, GrResourceProvider::Flags::kNoPendingIO);
        } else {
            texture = resourceProvider->createTexture(desc, SkBudgeted::kNo,
                                                      GrResourceProvider::Flags::kNoPendingIO);
        }
        return GrSurfaceProxy::LazyInstantiationResult(std::move(texture));
    };
    const GrBackendFormat format = caps->getBackendFormatFromColorType(grCT);
    auto lazyType = deinstantiate ? GrSurfaceProxy::LazyInstantiationType ::kDeinstantiate
                                  : GrSurfaceProxy::LazyInstantiationType ::kSingleUse;
    GrInternalSurfaceFlags flags = GrInternalSurfaceFlags::kNone;
    return proxyProvider->createLazyProxy(callback, format, desc, p.fOrigin, GrMipMapped::kNo,
                                          flags, p.fFit, p.fBudgeted, lazyType);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(LazyDeinstantiation, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    GrResourceProvider* resourceProvider = ctxInfo.grContext()->priv().resourceProvider();
    ProxyParams texParams;
    texParams.fFit = SkBackingFit::kExact;
    texParams.fOrigin = kTopLeft_GrSurfaceOrigin;
    texParams.fColorType = kRGBA_8888_SkColorType;
    texParams.fIsRT = false;
    texParams.fSampleCnt = 1;
    texParams.fSize = 100;
    texParams.fBudgeted = SkBudgeted::kNo;
    ProxyParams rtParams = texParams;
    rtParams.fIsRT = true;
    auto proxyProvider = context->priv().proxyProvider();
    auto caps = context->priv().caps();
    auto p0 = make_lazy(proxyProvider, caps, texParams, true);
    auto p1 = make_lazy(proxyProvider, caps, texParams, false);
    texParams.fFit = rtParams.fFit = SkBackingFit::kApprox;
    auto p2 = make_lazy(proxyProvider, caps, rtParams, true);
    auto p3 = make_lazy(proxyProvider, caps, rtParams, false);

    GrDeinstantiateProxyTracker deinstantiateTracker;
    {
        GrResourceAllocator alloc(resourceProvider, &deinstantiateTracker SkDEBUGCODE(, 1));
        alloc.addInterval(p0.get(), 0, 1, GrResourceAllocator::ActualUse::kNo);
        alloc.addInterval(p1.get(), 0, 1, GrResourceAllocator::ActualUse::kNo);
        alloc.addInterval(p2.get(), 0, 1, GrResourceAllocator::ActualUse::kNo);
        alloc.addInterval(p3.get(), 0, 1, GrResourceAllocator::ActualUse::kNo);
        alloc.incOps();
        alloc.markEndOfOpList(0);

        alloc.determineRecyclability();

        int startIndex, stopIndex;
        GrResourceAllocator::AssignError error;
        alloc.assign(&startIndex, &stopIndex, &error);
    }
    deinstantiateTracker.deinstantiateAllProxies();
    REPORTER_ASSERT(reporter, !p0->isInstantiated());
    REPORTER_ASSERT(reporter, p1->isInstantiated());
    REPORTER_ASSERT(reporter, !p2->isInstantiated());
    REPORTER_ASSERT(reporter, p3->isInstantiated());
}

// Set up so there are two opLists that need to be flushed but the resource allocator thinks
// it is over budget. The two opLists should be flushed separately and the opList indices
// returned from assign should be correct.
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ResourceAllocatorOverBudgetTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    const GrCaps* caps = context->priv().caps();
    GrProxyProvider* proxyProvider = context->priv().proxyProvider();
    GrResourceProvider* resourceProvider = context->priv().resourceProvider();

    int origMaxNum;
    size_t origMaxBytes;
    context->getResourceCacheLimits(&origMaxNum, &origMaxBytes);

    // Force the resource allocator to always believe it is over budget
    context->setResourceCacheLimits(0, 0);

    const ProxyParams params  = { 64, false, kRGBA_8888_SkColorType,
                                  SkBackingFit::kExact, 0, kTopLeft_GrSurfaceOrigin,
                                  SkBudgeted::kYes };

    {
        sk_sp<GrSurfaceProxy> p1 = make_deferred(proxyProvider, caps, params);
        sk_sp<GrSurfaceProxy> p2 = make_deferred(proxyProvider, caps, params);
        sk_sp<GrSurfaceProxy> p3 = make_deferred(proxyProvider, caps, params);
        sk_sp<GrSurfaceProxy> p4 = make_deferred(proxyProvider, caps, params);

        GrDeinstantiateProxyTracker deinstantiateTracker;
        GrResourceAllocator alloc(resourceProvider, &deinstantiateTracker SkDEBUGCODE(, 2));

        alloc.addInterval(p1.get(), 0, 0, GrResourceAllocator::ActualUse::kYes);
        alloc.incOps();
        alloc.addInterval(p2.get(), 1, 1, GrResourceAllocator::ActualUse::kYes);
        alloc.incOps();
        alloc.markEndOfOpList(0);

        alloc.addInterval(p3.get(), 2, 2, GrResourceAllocator::ActualUse::kYes);
        alloc.incOps();
        alloc.addInterval(p4.get(), 3, 3, GrResourceAllocator::ActualUse::kYes);
        alloc.incOps();
        alloc.markEndOfOpList(1);

        int startIndex, stopIndex;
        GrResourceAllocator::AssignError error;

        alloc.determineRecyclability();

        alloc.assign(&startIndex, &stopIndex, &error);
        REPORTER_ASSERT(reporter, GrResourceAllocator::AssignError::kNoError == error);
        REPORTER_ASSERT(reporter, 0 == startIndex && 1 == stopIndex);

        alloc.assign(&startIndex, &stopIndex, &error);
        REPORTER_ASSERT(reporter, GrResourceAllocator::AssignError::kNoError == error);
        REPORTER_ASSERT(reporter, 1 == startIndex && 2 == stopIndex);
    }

    context->setResourceCacheLimits(origMaxNum, origMaxBytes);
}
