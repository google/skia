/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Include here to ensure SK_SUPPORT_GPU is set correctly before it is examined.
#include "SkTypes.h"

#if SK_SUPPORT_GPU
#ifndef SK_DISABLE_DEFERRED_PROXIES
#include "Test.h"

#include "GrContextPriv.h"
#include "GrGpu.h"
#include "GrProxyProvider.h"
#include "GrResourceAllocator.h"
#include "GrResourceProvider.h"
#include "GrSurfaceProxyPriv.h"
#include "GrTest.h"
#include "GrTexture.h"
#include "GrTextureProxy.h"

struct ProxyParams {
    int             fSize;
    bool            fIsRT;
    GrPixelConfig   fConfig;
    SkBackingFit    fFit;
    int             fSampleCnt;
    GrSurfaceOrigin fOrigin;
    // TODO: do we care about mipmapping
};

static sk_sp<GrSurfaceProxy> make_deferred(GrProxyProvider* proxyProvider, const ProxyParams& p) {
    GrSurfaceDesc desc;
    desc.fFlags = p.fIsRT ? kRenderTarget_GrSurfaceFlag : kNone_GrSurfaceFlags;
    desc.fOrigin = p.fOrigin;
    desc.fWidth  = p.fSize;
    desc.fHeight = p.fSize;
    desc.fConfig = p.fConfig;
    desc.fSampleCnt = p.fSampleCnt;

    return proxyProvider->createProxy(desc, p.fFit, SkBudgeted::kNo);
}

static sk_sp<GrSurfaceProxy> make_backend(GrContext* context, const ProxyParams& p,
                                          GrBackendTexture* backendTex) {
    GrProxyProvider* proxyProvider = context->contextPriv().proxyProvider();
    GrGpu* gpu = context->contextPriv().getGpu();

    *backendTex = gpu->createTestingOnlyBackendTexture(nullptr, p.fSize, p.fSize,
                                                       p.fConfig, false,
                                                       GrMipMapped::kNo);

    return proxyProvider->createWrappedTextureProxy(*backendTex, p.fOrigin);
}

static void cleanup_backend(GrContext* context, GrBackendTexture* backendTex) {
    context->contextPriv().getGpu()->deleteTestingOnlyBackendTexture(backendTex);
}

// Basic test that two proxies with overlapping intervals and compatible descriptors are
// assigned different GrSurfaces.
static void overlap_test(skiatest::Reporter* reporter, GrResourceProvider* resourceProvider,
                         sk_sp<GrSurfaceProxy> p1, sk_sp<GrSurfaceProxy> p2,
                         bool expectedResult) {
    GrResourceAllocator alloc(resourceProvider);

    alloc.addInterval(p1.get(), 0, 4);
    alloc.addInterval(p2.get(), 1, 2);
    alloc.markEndOfOpList(0);

    int startIndex, stopIndex;
    GrResourceAllocator::AssignError error;
    alloc.assign(&startIndex, &stopIndex, &error);
    REPORTER_ASSERT(reporter, GrResourceAllocator::AssignError::kNoError == error);

    REPORTER_ASSERT(reporter, p1->priv().peekSurface());
    REPORTER_ASSERT(reporter, p2->priv().peekSurface());
    bool doTheBackingStoresMatch = p1->underlyingUniqueID() == p2->underlyingUniqueID();
    REPORTER_ASSERT(reporter, expectedResult == doTheBackingStoresMatch);
}

// Test various cases when two proxies do not have overlapping intervals.
// This mainly acts as a test of the ResourceAllocator's free pool.
static void non_overlap_test(skiatest::Reporter* reporter, GrResourceProvider* resourceProvider,
                             sk_sp<GrSurfaceProxy> p1, sk_sp<GrSurfaceProxy> p2,
                             bool expectedResult) {
    GrResourceAllocator alloc(resourceProvider);

    alloc.addInterval(p1.get(), 0, 2);
    alloc.addInterval(p2.get(), 3, 5);
    alloc.markEndOfOpList(0);

    int startIndex, stopIndex;
    GrResourceAllocator::AssignError error;
    alloc.assign(&startIndex, &stopIndex, &error);
    REPORTER_ASSERT(reporter, GrResourceAllocator::AssignError::kNoError == error);

    REPORTER_ASSERT(reporter, p1->priv().peekSurface());
    REPORTER_ASSERT(reporter, p2->priv().peekSurface());
    bool doTheBackingStoresMatch = p1->underlyingUniqueID() == p2->underlyingUniqueID();
    REPORTER_ASSERT(reporter, expectedResult == doTheBackingStoresMatch);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ResourceAllocatorTest, reporter, ctxInfo) {
    GrProxyProvider* proxyProvider = ctxInfo.grContext()->contextPriv().proxyProvider();
    GrResourceProvider* resourceProvider = ctxInfo.grContext()->contextPriv().resourceProvider();

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

    const GrPixelConfig kRGBA = kRGBA_8888_GrPixelConfig;
    const GrPixelConfig kBGRA = kBGRA_8888_GrPixelConfig;

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
        sk_sp<GrSurfaceProxy> p1 = make_deferred(proxyProvider, test.fP1);
        sk_sp<GrSurfaceProxy> p2 = make_deferred(proxyProvider, test.fP2);
        overlap_test(reporter, resourceProvider,
                     std::move(p1), std::move(p2), test.fExpectation);
    }

    int k2 = ctxInfo.grContext()->caps()->getSampleCount(2, kRGBA);
    int k4 = ctxInfo.grContext()->caps()->getSampleCount(4, kRGBA);

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
        sk_sp<GrSurfaceProxy> p1 = make_deferred(proxyProvider, test.fP1);
        sk_sp<GrSurfaceProxy> p2 = make_deferred(proxyProvider, test.fP2);
        if (!p1 || !p2) {
            continue; // creation can fail (i.e., for msaa4 on iOS)
        }
        non_overlap_test(reporter, resourceProvider,
                         std::move(p1), std::move(p2), test.fExpectation);
    }

    {
        // Wrapped backend textures should never be reused
        TestCase t[1] = {
            { { 64, kNotRT, kRGBA, kE, 0, kTL }, { 64, kNotRT, kRGBA, kE, 0, kTL }, kDontShare }
        };

        GrBackendTexture backEndTex;
        sk_sp<GrSurfaceProxy> p1 = make_backend(ctxInfo.grContext(), t[0].fP1, &backEndTex);
        sk_sp<GrSurfaceProxy> p2 = make_deferred(proxyProvider, t[0].fP2);
        non_overlap_test(reporter, resourceProvider,
                         std::move(p1), std::move(p2), t[0].fExpectation);
        cleanup_backend(ctxInfo.grContext(), &backEndTex);
    }
}

#endif
#endif
