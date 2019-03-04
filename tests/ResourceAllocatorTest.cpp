/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"

#include "Test.h"

#include "GrContextPriv.h"
#include "GrGpu.h"
#include "GrProxyProvider.h"
#include "GrResourceAllocator.h"
#include "GrResourceProvider.h"
#include "GrSurfaceProxyPriv.h"
#include "GrTexture.h"
#include "GrTextureProxy.h"

#include "SkSurface.h"

struct ProxyParams {
    int             fSize;
    bool            fIsRT;
    SkColorType     fColorType;
    SkBackingFit    fFit;
    int             fSampleCnt;
    GrSurfaceOrigin fOrigin;
    // TODO: do we care about mipmapping
};

static GrSurfaceProxy* make_deferred(GrProxyProvider* proxyProvider, const GrCaps* caps,
                                     const ProxyParams& p) {
    GrColorType grCT = SkColorTypeToGrColorType(p.fColorType);
    GrPixelConfig config = GrColorTypeToPixelConfig(grCT, GrSRGBEncoded::kNo);

    GrSurfaceDesc desc;
    desc.fFlags = p.fIsRT ? kRenderTarget_GrSurfaceFlag : kNone_GrSurfaceFlags;
    desc.fWidth  = p.fSize;
    desc.fHeight = p.fSize;
    desc.fConfig = config;
    desc.fSampleCnt = p.fSampleCnt;

    const GrBackendFormat format = caps->getBackendFormatFromColorType(p.fColorType);

    auto tmp = proxyProvider->createProxy(format, desc, p.fOrigin, p.fFit, SkBudgeted::kNo);
    if (!tmp) {
        return nullptr;
    }
    GrSurfaceProxy* ret = tmp.release();

    // Add a read to keep the proxy around but unref it so its backing surfaces can be recycled
    ret->addPendingRead();
    ret->unref();
    return ret;
}

static GrSurfaceProxy* make_backend(GrContext* context, const ProxyParams& p,
                                    GrBackendTexture* backendTex) {
    GrProxyProvider* proxyProvider = context->priv().proxyProvider();
    GrGpu* gpu = context->priv().getGpu();

    *backendTex = gpu->createTestingOnlyBackendTexture(nullptr, p.fSize, p.fSize,
                                                       p.fColorType, false,
                                                       GrMipMapped::kNo);
    if (!backendTex->isValid()) {
        return nullptr;
    }

    auto tmp = proxyProvider->wrapBackendTexture(*backendTex, p.fOrigin, kBorrow_GrWrapOwnership,
                                                 GrWrapCacheable::kNo, kRead_GrIOType);
    if (!tmp) {
        return nullptr;
    }
    GrSurfaceProxy* ret = tmp.release();

    // Add a read to keep the proxy around but unref it so its backing surfaces can be recycled
    ret->addPendingRead();
    ret->unref();
    return ret;
}

static void cleanup_backend(GrContext* context, const GrBackendTexture& backendTex) {
    context->priv().getGpu()->deleteTestingOnlyBackendTexture(backendTex);
}

// Basic test that two proxies with overlapping intervals and compatible descriptors are
// assigned different GrSurfaces.
static void overlap_test(skiatest::Reporter* reporter, GrResourceProvider* resourceProvider,
                         GrSurfaceProxy* p1, GrSurfaceProxy* p2, bool expectedResult) {
    GrResourceAllocator alloc(resourceProvider);

    alloc.addInterval(p1, 0, 4);
    alloc.addInterval(p2, 1, 2);
    alloc.markEndOfOpList(0);

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
                             GrSurfaceProxy* p1, GrSurfaceProxy* p2,
                             bool expectedResult) {
    GrResourceAllocator alloc(resourceProvider);

    alloc.addInterval(p1, 0, 2);
    alloc.addInterval(p2, 3, 5);
    alloc.markEndOfOpList(0);

    int startIndex, stopIndex;
    GrResourceAllocator::AssignError error;
    alloc.assign(&startIndex, &stopIndex, &error);
    REPORTER_ASSERT(reporter, GrResourceAllocator::AssignError::kNoError == error);

    REPORTER_ASSERT(reporter, p1->peekSurface());
    REPORTER_ASSERT(reporter, p2->peekSurface());
    bool doTheBackingStoresMatch = p1->underlyingUniqueID() == p2->underlyingUniqueID();
    REPORTER_ASSERT(reporter, expectedResult == doTheBackingStoresMatch);
}

bool GrResourceProvider::testingOnly_setExplicitlyAllocateGPUResources(bool newValue) {
    bool oldValue = fExplicitlyAllocateGPUResources;
    fExplicitlyAllocateGPUResources = newValue;
    return oldValue;
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ResourceAllocatorTest, reporter, ctxInfo) {
    const GrCaps* caps = ctxInfo.grContext()->priv().caps();
    GrProxyProvider* proxyProvider = ctxInfo.grContext()->priv().proxyProvider();
    GrResourceProvider* resourceProvider = ctxInfo.grContext()->priv().resourceProvider();

    bool orig = resourceProvider->testingOnly_setExplicitlyAllocateGPUResources(true);

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

    //--------------------------------------------------------------------------------------------
    TestCase gOverlappingTests[] = {
        //----------------------------------------------------------------------------------------
        // Two proxies with overlapping intervals and compatible descriptors should never share
        // RT version
        { { 64,    kRT, kRGBA, kA, 0, kTL }, { 64,    kRT, kRGBA, kA, 0, kTL }, kDontShare },
        // non-RT version
        { { 64, kNotRT, kRGBA, kA, 0, kTL }, { 64, kNotRT, kRGBA, kA, 0, kTL }, kDontShare },
    };

    for (auto test : gOverlappingTests) {
        GrSurfaceProxy* p1 = make_deferred(proxyProvider, caps, test.fP1);
        GrSurfaceProxy* p2 = make_deferred(proxyProvider, caps, test.fP2);
        overlap_test(reporter, resourceProvider, p1, p2, test.fExpectation);
        p1->completedRead();
        p2->completedRead();
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
        { { 64,    kRT, kRGBA, kA, 0, kTL }, { 64,    kRT, kRGBA, kA, 0, kTL }, kShare },
        { { 64, kNotRT, kRGBA, kA, 0, kTL }, { 64, kNotRT, kRGBA, kA, 0, kTL }, kConditionallyShare },
        // diffs sizes but still approx
        { { 64,    kRT, kRGBA, kA, 0, kTL }, { 50,    kRT, kRGBA, kA, 0, kTL }, kShare },
        { { 64, kNotRT, kRGBA, kA, 0, kTL }, { 50, kNotRT, kRGBA, kA, 0, kTL }, kConditionallyShare },
        // sames sizes but exact
        { { 64,    kRT, kRGBA, kE, 0, kTL }, { 64,    kRT, kRGBA, kE, 0, kTL }, kShare },
        { { 64, kNotRT, kRGBA, kE, 0, kTL }, { 64, kNotRT, kRGBA, kE, 0, kTL }, kConditionallyShare },
        //----------------------------------------------------------------------------------------
        // Two non-overlapping intervals w/ different exact sizes should not share
        { { 56,    kRT, kRGBA, kE, 0, kTL }, { 54,    kRT, kRGBA, kE, 0, kTL }, kDontShare },
        // Two non-overlapping intervals w/ _very different_ approx sizes should not share
        { { 255,   kRT, kRGBA, kA, 0, kTL }, { 127,   kRT, kRGBA, kA, 0, kTL }, kDontShare },
        // Two non-overlapping intervals w/ different MSAA sample counts should not share
        { { 64,    kRT, kRGBA, kA, k2, kTL },{ 64,    kRT, kRGBA, kA, k4, kTL}, k2 == k4 },
        // Two non-overlapping intervals w/ different configs should not share
        { { 64,    kRT, kRGBA, kA, 0, kTL }, { 64,    kRT, kBGRA, kA, 0, kTL }, kDontShare },
        // Two non-overlapping intervals w/ different RT classifications should never share
        { { 64,    kRT, kRGBA, kA, 0, kTL }, { 64, kNotRT, kRGBA, kA, 0, kTL }, kDontShare },
        { { 64, kNotRT, kRGBA, kA, 0, kTL }, { 64,    kRT, kRGBA, kA, 0, kTL }, kDontShare },
        // Two non-overlapping intervals w/ different origins should share
        { { 64,    kRT, kRGBA, kA, 0, kTL }, { 64,    kRT, kRGBA, kA, 0, kBL }, kShare },
    };

    for (auto test : gNonOverlappingTests) {
        GrSurfaceProxy* p1 = make_deferred(proxyProvider, caps, test.fP1);
        GrSurfaceProxy* p2 = make_deferred(proxyProvider, caps, test.fP2);

        if (!p1 || !p2) {
            continue; // creation can fail (i.e., for msaa4 on iOS)
        }

        non_overlap_test(reporter, resourceProvider, p1, p2, test.fExpectation);

        p1->completedRead();
        p2->completedRead();
    }

    {
        // Wrapped backend textures should never be reused
        TestCase t[1] = {
            { { 64, kNotRT, kRGBA, kE, 0, kTL }, { 64, kNotRT, kRGBA, kE, 0, kTL }, kDontShare }
        };

        GrBackendTexture backEndTex;
        GrSurfaceProxy* p1 = make_backend(ctxInfo.grContext(), t[0].fP1, &backEndTex);
        GrSurfaceProxy* p2 = make_deferred(proxyProvider, caps, t[0].fP2);

        non_overlap_test(reporter, resourceProvider, p1, p2, t[0].fExpectation);

        p1->completedRead();
        p2->completedRead();

        cleanup_backend(ctxInfo.grContext(), backEndTex);
    }

    resourceProvider->testingOnly_setExplicitlyAllocateGPUResources(orig);
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
    GrResourceProvider* resourceProvider = ctxInfo.grContext()->priv().resourceProvider();

    int maxNum;
    size_t maxBytes;
    context->getResourceCacheLimits(&maxNum, &maxBytes);

    bool orig = resourceProvider->testingOnly_setExplicitlyAllocateGPUResources(true);
    context->setResourceCacheLimits(0, 0); // We'll always be overbudget

    draw(context);
    draw(context);
    draw(context);
    draw(context);
    context->flush();

    context->setResourceCacheLimits(maxNum, maxBytes);
    resourceProvider->testingOnly_setExplicitlyAllocateGPUResources(orig);
}

sk_sp<GrSurfaceProxy> make_lazy(GrProxyProvider* proxyProvider, const GrCaps* caps,
                                const ProxyParams& p) {
    GrColorType grCT = SkColorTypeToGrColorType(p.fColorType);
    GrPixelConfig config = GrColorTypeToPixelConfig(grCT, GrSRGBEncoded::kNo);

    GrSurfaceDesc desc;
    desc.fFlags = p.fIsRT ? kRenderTarget_GrSurfaceFlag : kNone_GrSurfaceFlags;
    desc.fWidth = p.fSize;
    desc.fHeight = p.fSize;
    desc.fConfig = config;
    desc.fSampleCnt = p.fSampleCnt;

    SkBackingFit fit = p.fFit;
    auto callback = [fit, desc](GrResourceProvider* resourceProvider) -> sk_sp<GrSurface> {
        if (!resourceProvider) {
            return nullptr;
        }
        if (fit == SkBackingFit::kApprox) {
            return resourceProvider->createApproxTexture(desc, GrResourceProvider::Flags::kNone);
        } else {
            return resourceProvider->createTexture(desc, SkBudgeted::kNo);
        }
    };
    const GrBackendFormat format = caps->getBackendFormatFromColorType(p.fColorType);
    auto lazyType = GrSurfaceProxy::LazyInstantiationType ::kSingleUse;
    GrInternalSurfaceFlags flags = GrInternalSurfaceFlags::kNone;
    return proxyProvider->createLazyProxy(callback, format, desc, p.fOrigin, GrMipMapped::kNo,
                                          flags, p.fFit, SkBudgeted::kNo, lazyType);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(LazyDeinstantiation, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    GrResourceProvider* resourceProvider = ctxInfo.grContext()->priv().resourceProvider();
    for (auto explicitlyAllocating : {false, true}) {
        resourceProvider->testingOnly_setExplicitlyAllocateGPUResources(explicitlyAllocating);
        ProxyParams texParams;
        texParams.fFit = SkBackingFit::kExact;
        texParams.fOrigin = kTopLeft_GrSurfaceOrigin;
        texParams.fColorType = kRGBA_8888_SkColorType;
        texParams.fIsRT = false;
        texParams.fSampleCnt = 1;
        texParams.fSize = 100;
        ProxyParams rtParams = texParams;
        rtParams.fIsRT = true;
        auto proxyProvider = context->priv().proxyProvider();
        auto caps = context->priv().caps();
        auto p0 = make_lazy(proxyProvider, caps, texParams);
        texParams.fFit = rtParams.fFit = SkBackingFit::kApprox;
        auto p1 = make_lazy(proxyProvider, caps, rtParams);

        {
            GrResourceAllocator alloc(resourceProvider);
            alloc.addInterval(p0.get(), 0, 1);
            alloc.addInterval(p1.get(), 0, 1);
            alloc.markEndOfOpList(0);
            int startIndex, stopIndex;
            GrResourceAllocator::AssignError error;
            alloc.assign(&startIndex, &stopIndex, &error);
        }
        REPORTER_ASSERT(reporter, p0->isInstantiated());
        REPORTER_ASSERT(reporter, p1->isInstantiated());
    }
}
