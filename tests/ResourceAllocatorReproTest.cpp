/*
 * Copyright 2019 Google LLC
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
    GrRenderable    fRenderable;
    GrColorType     fColorType;
    SkBackingFit    fFit;
    int             fSampleCnt;
    GrSurfaceOrigin fOrigin;
    SkBudgeted      fBudgeted;
    // TODO: do we care about mipmapping
};

static sk_sp<GrSurfaceProxy> make_deferred(GrProxyProvider* proxyProvider, const GrCaps* caps,
    const ProxyParams& p) {
    GrPixelConfig config = GrColorTypeToPixelConfig(p.fColorType);

    GrSurfaceDesc desc;
    desc.fWidth = p.fSize;
    desc.fHeight = p.fSize;
    desc.fConfig = config;

    const GrBackendFormat format = caps->getDefaultBackendFormat(p.fColorType, p.fRenderable);

    return proxyProvider->createProxy(format, desc, p.fRenderable, p.fSampleCnt, p.fOrigin, p.fFit,
        p.fBudgeted, GrProtected::kNo);
}

static sk_sp<GrSurfaceProxy> make_backend(GrContext* context, const ProxyParams& p,
    GrBackendTexture* backendTex) {
    GrProxyProvider* proxyProvider = context->priv().proxyProvider();

    SkColorType skColorType = GrColorTypeToSkColorType(p.fColorType);
    SkASSERT(SkColorType::kUnknown_SkColorType != skColorType);

    *backendTex = context->createBackendTexture(p.fSize, p.fSize, skColorType,
        SkColors::kTransparent,
        GrMipMapped::kNo, GrRenderable::kNo,
        GrProtected::kNo);
    if (!backendTex->isValid()) {
        return nullptr;
    }

    return proxyProvider->wrapBackendTexture(*backendTex, p.fColorType, p.fOrigin,
        kBorrow_GrWrapOwnership, GrWrapCacheable::kNo,
        kRead_GrIOType);
}

static void cleanup_backend(GrContext* context, const GrBackendTexture& backendTex) {
    context->deleteBackendTexture(backendTex);
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

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ResourceAllocatorReproTest, reporter, ctxInfo) {
    const GrCaps* caps = ctxInfo.grContext()->priv().caps();
    GrProxyProvider* proxyProvider = ctxInfo.grContext()->priv().proxyProvider();
    GrResourceProvider* resourceProvider = ctxInfo.grContext()->priv().resourceProvider();

    struct TestCase {
        ProxyParams   fP1;
        ProxyParams   fP2;
        bool          fExpectation;
    };

    constexpr GrRenderable kRT = GrRenderable::kYes;
    constexpr GrRenderable kNotRT = GrRenderable::kNo;

    constexpr bool kShare = true;
    constexpr bool kDontShare = false;
    // Non-RT GrSurfaces are never recycled on some platforms.
    bool kConditionallyShare = resourceProvider->caps()->reuseScratchTextures();

    const GrColorType kRGBA = GrColorType::kRGBA_8888;
    const GrColorType kBGRA = GrColorType::kBGRA_8888;

    const SkBackingFit kE = SkBackingFit::kExact;
    const SkBackingFit kA = SkBackingFit::kApprox;

    const GrSurfaceOrigin kTL = kTopLeft_GrSurfaceOrigin;
    const GrSurfaceOrigin kBL = kBottomLeft_GrSurfaceOrigin;

    const SkBudgeted kNotB = SkBudgeted::kNo;

    int k2 = ctxInfo.grContext()->priv().caps()->getRenderTargetSampleCount(
        2, kRGBA_8888_GrPixelConfig);
    int k4 = ctxInfo.grContext()->priv().caps()->getRenderTargetSampleCount(
        4, kRGBA_8888_GrPixelConfig);

    //--------------------------------------------------------------------------------------------
    TestCase gNonOverlappingTests[] = {
        //----------------------------------------------------------------------------------------
        // Two non-overlapping intervals w/ compatible proxies should share
        // both same size & approx
        { { 64,    kRT, kRGBA, kA, 1, kTL, kNotB }, { 64,    kRT, kRGBA, kA, 1, kTL, kNotB }, kShare },
        { { 64, kNotRT, kRGBA, kA, 1, kTL, kNotB }, { 64, kNotRT, kRGBA, kA, 1, kTL, kNotB }, kConditionallyShare },
        // diffs sizes but still approx
        { { 64,    kRT, kRGBA, kA, 1, kTL, kNotB }, { 50,    kRT, kRGBA, kA, 1, kTL, kNotB }, kShare },
        { { 64, kNotRT, kRGBA, kA, 1, kTL, kNotB }, { 50, kNotRT, kRGBA, kA, 1, kTL, kNotB }, kConditionallyShare },
        // sames sizes but exact
        { { 64,    kRT, kRGBA, kE, 1, kTL, kNotB }, { 64,    kRT, kRGBA, kE, 1, kTL, kNotB }, kShare },
        { { 64, kNotRT, kRGBA, kE, 1, kTL, kNotB }, { 64, kNotRT, kRGBA, kE, 1, kTL, kNotB }, kConditionallyShare },
        //----------------------------------------------------------------------------------------
        // Two non-overlapping intervals w/ different exact sizes should not share
        { { 56,    kRT, kRGBA, kE, 1, kTL, kNotB }, { 54,    kRT, kRGBA, kE, 1, kTL, kNotB }, kDontShare },
        // Two non-overlapping intervals w/ _very different_ approx sizes should not share
        { { 255,   kRT, kRGBA, kA, 1, kTL, kNotB }, { 127,   kRT, kRGBA, kA, 1, kTL, kNotB }, kDontShare },
        // Two non-overlapping intervals w/ different MSAA sample counts should not share
        { { 64,    kRT, kRGBA, kA, k2, kTL, kNotB },{ 64,    kRT, kRGBA, kA, k4,kTL, kNotB}, k2 == k4 },
        // Two non-overlapping intervals w/ different configs should not share
        { { 64,    kRT, kRGBA, kA, 1, kTL, kNotB }, { 64,    kRT, kBGRA, kA, 1, kTL, kNotB }, kDontShare },
        // Two non-overlapping intervals w/ different RT classifications should never share
        { { 64,    kRT, kRGBA, kA, 1, kTL, kNotB }, { 64, kNotRT, kRGBA, kA, 1, kTL, kNotB }, kDontShare },
        { { 64, kNotRT, kRGBA, kA, 1, kTL, kNotB }, { 64,    kRT, kRGBA, kA, 1, kTL, kNotB }, kDontShare },
        // Two non-overlapping intervals w/ different origins should share
        { { 64,    kRT, kRGBA, kA, 1, kTL, kNotB }, { 64,    kRT, kRGBA, kA, 1, kBL, kNotB }, kShare },
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
            { { 64, kNotRT, kRGBA, kE, 1, kTL, kNotB }, { 64, kNotRT, kRGBA, kE, 1, kTL, kNotB }, kDontShare }
        };

        GrBackendTexture backEndTex;
        sk_sp<GrSurfaceProxy> p1 = make_backend(ctxInfo.grContext(), t[0].fP1, &backEndTex);
        sk_sp<GrSurfaceProxy> p2 = make_deferred(proxyProvider, caps, t[0].fP2);

        non_overlap_test(reporter, resourceProvider, std::move(p1), std::move(p2),
            t[0].fExpectation);

        cleanup_backend(ctxInfo.grContext(), backEndTex);
    }
}
