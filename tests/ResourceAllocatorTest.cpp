/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrResourceAllocator.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrSurfaceProxyPriv.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/GrTextureProxy.h"
#include "tests/Test.h"
#include "tools/gpu/ManagedBackendTexture.h"

struct ProxyParams {
    int             fSize;
    GrRenderable    fRenderable;
    GrColorType     fColorType;
    SkBackingFit    fFit;
    int             fSampleCnt;
    SkBudgeted      fBudgeted;
    // TODO: do we care about mipmapping
};

static sk_sp<GrSurfaceProxy> make_deferred(GrProxyProvider* proxyProvider, const GrCaps* caps,
                                           const ProxyParams& p) {
    const GrBackendFormat format = caps->getDefaultBackendFormat(p.fColorType, p.fRenderable);
    return proxyProvider->createProxy(format, {p.fSize, p.fSize}, p.fRenderable, p.fSampleCnt,
                                      GrMipmapped::kNo, p.fFit, p.fBudgeted, GrProtected::kNo);
}

static sk_sp<GrSurfaceProxy> make_backend(GrDirectContext* dContext, const ProxyParams& p) {
    GrProxyProvider* proxyProvider = dContext->priv().proxyProvider();

    SkColorType skColorType = GrColorTypeToSkColorType(p.fColorType);
    SkASSERT(SkColorType::kUnknown_SkColorType != skColorType);

    auto mbet = sk_gpu_test::ManagedBackendTexture::MakeWithoutData(
            dContext, p.fSize, p.fSize, skColorType, GrMipmapped::kNo, GrRenderable::kNo);

    if (!mbet) {
        return nullptr;
    }

    return proxyProvider->wrapBackendTexture(mbet->texture(),
                                             kBorrow_GrWrapOwnership,
                                             GrWrapCacheable::kNo,
                                             kRead_GrIOType,
                                             mbet->refCountedCallback());
}

// Basic test that two proxies with overlapping intervals and compatible descriptors are
// assigned different GrSurfaces.
static void overlap_test(skiatest::Reporter* reporter, GrResourceProvider* resourceProvider,
                         sk_sp<GrSurfaceProxy> p1, sk_sp<GrSurfaceProxy> p2,
                         bool expectedResult) {
    GrResourceAllocator alloc(resourceProvider SkDEBUGCODE(, 1));

    alloc.addInterval(p1.get(), 0, 4, GrResourceAllocator::ActualUse::kYes);
    alloc.incOps();
    alloc.addInterval(p2.get(), 1, 2, GrResourceAllocator::ActualUse::kYes);
    alloc.incOps();
    alloc.markEndOfOpsTask(0);

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
    GrResourceAllocator alloc(resourceProvider SkDEBUGCODE(, 1));

    alloc.incOps();
    alloc.incOps();
    alloc.incOps();
    alloc.incOps();
    alloc.incOps();
    alloc.incOps();

    alloc.addInterval(p1.get(), 0, 2, GrResourceAllocator::ActualUse::kYes);
    alloc.addInterval(p2.get(), 3, 5, GrResourceAllocator::ActualUse::kYes);
    alloc.markEndOfOpsTask(0);

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
    auto direct = ctxInfo.directContext();
    const GrCaps* caps = direct->priv().caps();
    GrProxyProvider* proxyProvider = direct->priv().proxyProvider();
    GrResourceProvider* resourceProvider = direct->priv().resourceProvider();

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
    const GrColorType kAlpha = GrColorType::kAlpha_8;

    const SkBackingFit kE = SkBackingFit::kExact;
    const SkBackingFit kA = SkBackingFit::kApprox;

    const SkBudgeted kNotB = SkBudgeted::kNo;

    //--------------------------------------------------------------------------------------------
    TestCase gOverlappingTests[] = {
            //----------------------------------------------------------------------------------------
            // Two proxies with overlapping intervals and compatible descriptors should never share
            // RT version
            {{64, kRT, kRGBA, kA, 1, kNotB}, {64, kRT, kRGBA, kA, 1, kNotB}, kDontShare},
            // non-RT version
            {{64, kNotRT, kRGBA, kA, 1, kNotB}, {64, kNotRT, kRGBA, kA, 1, kNotB}, kDontShare},
    };

    for (auto test : gOverlappingTests) {
        sk_sp<GrSurfaceProxy> p1 = make_deferred(proxyProvider, caps, test.fP1);
        sk_sp<GrSurfaceProxy> p2 = make_deferred(proxyProvider, caps, test.fP2);
        overlap_test(reporter, resourceProvider, std::move(p1), std::move(p2), test.fExpectation);
    }

    auto beFormat = caps->getDefaultBackendFormat(GrColorType::kRGBA_8888, GrRenderable::kYes);
    int k2 = direct->priv().caps()->getRenderTargetSampleCount(2, beFormat);
    int k4 = direct->priv().caps()->getRenderTargetSampleCount(4, beFormat);

    //--------------------------------------------------------------------------------------------
    TestCase gNonOverlappingTests[] = {
            //----------------------------------------------------------------------------------------
            // Two non-overlapping intervals w/ compatible proxies should share
            // both same size & approx
            {{64, kRT, kRGBA, kA, 1, kNotB}, {64, kRT, kRGBA, kA, 1, kNotB}, kShare},
            {{64, kNotRT, kRGBA, kA, 1, kNotB},
             {64, kNotRT, kRGBA, kA, 1, kNotB},
             kConditionallyShare},
            // diffs sizes but still approx
            {{64, kRT, kRGBA, kA, 1, kNotB}, {50, kRT, kRGBA, kA, 1, kNotB}, kShare},
            {{64, kNotRT, kRGBA, kA, 1, kNotB},
             {50, kNotRT, kRGBA, kA, 1, kNotB},
             kConditionallyShare},
            // sames sizes but exact
            {{64, kRT, kRGBA, kE, 1, kNotB}, {64, kRT, kRGBA, kE, 1, kNotB}, kShare},
            {{64, kNotRT, kRGBA, kE, 1, kNotB},
             {64, kNotRT, kRGBA, kE, 1, kNotB},
             kConditionallyShare},
            //----------------------------------------------------------------------------------------
            // Two non-overlapping intervals w/ different exact sizes should not share
            {{56, kRT, kRGBA, kE, 1, kNotB}, {54, kRT, kRGBA, kE, 1, kNotB}, kDontShare},
            // Two non-overlapping intervals w/ _very different_ approx sizes should not share
            {{255, kRT, kRGBA, kA, 1, kNotB}, {127, kRT, kRGBA, kA, 1, kNotB}, kDontShare},
            // Two non-overlapping intervals w/ different MSAA sample counts should not share
            {{64, kRT, kRGBA, kA, k2, kNotB}, {64, kRT, kRGBA, kA, k4, kNotB}, k2 == k4},
            // Two non-overlapping intervals w/ different configs should not share
            {{64, kRT, kRGBA, kA, 1, kNotB}, {64, kRT, kAlpha, kA, 1, kNotB}, kDontShare},
            // Two non-overlapping intervals w/ different RT classifications should never share
            {{64, kRT, kRGBA, kA, 1, kNotB}, {64, kNotRT, kRGBA, kA, 1, kNotB}, kDontShare},
            {{64, kNotRT, kRGBA, kA, 1, kNotB}, {64, kRT, kRGBA, kA, 1, kNotB}, kDontShare},
            // Two non-overlapping intervals w/ different origins should share
            {{64, kRT, kRGBA, kA, 1, kNotB}, {64, kRT, kRGBA, kA, 1, kNotB}, kShare},
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
                {{64, kNotRT, kRGBA, kE, 1, kNotB}, {64, kNotRT, kRGBA, kE, 1, kNotB}, kDontShare}};

        sk_sp<GrSurfaceProxy> p1 = make_backend(direct, t[0].fP1);
        sk_sp<GrSurfaceProxy> p2 = make_deferred(proxyProvider, caps, t[0].fP2);

        non_overlap_test(reporter, resourceProvider, std::move(p1), std::move(p2),
                         t[0].fExpectation);
    }
}

static void draw(GrRecordingContext* rContext) {
    SkImageInfo ii = SkImageInfo::Make(1024, 1024, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    sk_sp<SkSurface> s = SkSurface::MakeRenderTarget(rContext, SkBudgeted::kYes,
                                                     ii, 1, kTopLeft_GrSurfaceOrigin, nullptr);

    SkCanvas* c = s->getCanvas();

    c->clear(SK_ColorBLACK);
}


DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ResourceAllocatorStressTest, reporter, ctxInfo) {
    auto context = ctxInfo.directContext();

    size_t maxBytes = context->getResourceCacheLimit();

    context->setResourceCacheLimit(0); // We'll always be overbudget

    draw(context);
    draw(context);
    draw(context);
    draw(context);
    context->flushAndSubmit();

    context->setResourceCacheLimit(maxBytes);
}

// Set up so there are two opsTasks that need to be flushed but the resource allocator thinks
// it is over budget. The two opsTasks should be flushed separately and the opsTask indices
// returned from assign should be correct.
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ResourceAllocatorOverBudgetTest, reporter, ctxInfo) {
    auto context = ctxInfo.directContext();
    const GrCaps* caps = context->priv().caps();
    GrProxyProvider* proxyProvider = context->priv().proxyProvider();
    GrResourceProvider* resourceProvider = context->priv().resourceProvider();

    size_t origMaxBytes = context->getResourceCacheLimit();

    // Force the resource allocator to always believe it is over budget
    context->setResourceCacheLimit(0);

    const ProxyParams params = {
            64, GrRenderable::kNo, GrColorType::kRGBA_8888, SkBackingFit::kExact,
            1,  SkBudgeted::kYes};

    {
        sk_sp<GrSurfaceProxy> p1 = make_deferred(proxyProvider, caps, params);
        sk_sp<GrSurfaceProxy> p2 = make_deferred(proxyProvider, caps, params);
        sk_sp<GrSurfaceProxy> p3 = make_deferred(proxyProvider, caps, params);
        sk_sp<GrSurfaceProxy> p4 = make_deferred(proxyProvider, caps, params);

        GrResourceAllocator alloc(resourceProvider SkDEBUGCODE(, 2));

        alloc.addInterval(p1.get(), 0, 0, GrResourceAllocator::ActualUse::kYes);
        alloc.incOps();
        alloc.addInterval(p2.get(), 1, 1, GrResourceAllocator::ActualUse::kYes);
        alloc.incOps();
        alloc.markEndOfOpsTask(0);

        alloc.addInterval(p3.get(), 2, 2, GrResourceAllocator::ActualUse::kYes);
        alloc.incOps();
        alloc.addInterval(p4.get(), 3, 3, GrResourceAllocator::ActualUse::kYes);
        alloc.incOps();
        alloc.markEndOfOpsTask(1);

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

    context->setResourceCacheLimit(origMaxBytes);
}

// This test is used to make sure we are tracking the current task index during the assign call in
// the GrResourceAllocator. Specifically we can fall behind if we have intervals that don't
// use the allocator. In this case we need to possibly increment the fCurOpsTaskIndex multiple times
// to get in back in sync. We had a bug where we'd only every increment the index by one,
// http://crbug.com/996610.
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ResourceAllocatorCurOpsTaskIndexTest,
                                   reporter, ctxInfo) {
    auto context = ctxInfo.directContext();
    const GrCaps* caps = context->priv().caps();
    GrProxyProvider* proxyProvider = context->priv().proxyProvider();
    GrResourceProvider* resourceProvider = context->priv().resourceProvider();

    size_t origMaxBytes = context->getResourceCacheLimit();

    // Force the resource allocator to always believe it is over budget
    context->setResourceCacheLimit(0);

    ProxyParams params;
    params.fFit = SkBackingFit::kExact;
    params.fColorType = GrColorType::kRGBA_8888;
    params.fRenderable = GrRenderable::kYes;
    params.fSampleCnt = 1;
    params.fSize = 100;
    params.fBudgeted = SkBudgeted::kYes;

    sk_sp<GrSurfaceProxy> proxy1 = make_deferred(proxyProvider, caps, params);
    if (!proxy1) {
        return;
    }
    sk_sp<GrSurfaceProxy> proxy2 = make_deferred(proxyProvider, caps, params);
    if (!proxy2) {
        return;
    }

    // Wrapped proxy that will be ignored by the resourceAllocator. We use this to try and get the
    // resource allocator fCurOpsTaskIndex to fall behind what it really should be.
    sk_sp<GrSurfaceProxy> proxyWrapped = make_backend(context, params);
    if (!proxyWrapped) {
        return;
    }

    // Same as above, but we actually need to have at least two intervals that don't go through the
    // resource allocator to expose the index bug.
    sk_sp<GrSurfaceProxy> proxyWrapped2 = make_backend(context, params);
    if (!proxyWrapped2) {
        return;
    }

    GrResourceAllocator alloc(resourceProvider SkDEBUGCODE(, 4));

    alloc.addInterval(proxyWrapped.get(), 0, 0, GrResourceAllocator::ActualUse::kYes);
    alloc.incOps();
    alloc.markEndOfOpsTask(0);

    alloc.addInterval(proxyWrapped2.get(), 1, 1, GrResourceAllocator::ActualUse::kYes);
    alloc.incOps();
    alloc.markEndOfOpsTask(1);

    alloc.addInterval(proxy1.get(), 2, 2, GrResourceAllocator::ActualUse::kYes);
    alloc.incOps();
    alloc.markEndOfOpsTask(2);

    // We want to force the resource allocator to do a intermediateFlush for the previous interval.
    // But if it is the resource allocator is at the of its list of intervals it skips the
    // intermediate flush call, so we add another interval here so it is not skipped.
    alloc.addInterval(proxy2.get(), 3, 3, GrResourceAllocator::ActualUse::kYes);
    alloc.incOps();
    alloc.markEndOfOpsTask(3);

    int startIndex, stopIndex;
    GrResourceAllocator::AssignError error;

    alloc.determineRecyclability();

    alloc.assign(&startIndex, &stopIndex, &error);
    REPORTER_ASSERT(reporter, GrResourceAllocator::AssignError::kNoError == error);
    // The original bug in the allocator here would return a stopIndex of 2 since it would have only
    // incremented its fCurOpsTaskIndex once instead of the needed two times to skip the first two
    // unused intervals.
    REPORTER_ASSERT(reporter, 0 == startIndex && 3 == stopIndex);

    alloc.assign(&startIndex, &stopIndex, &error);
    REPORTER_ASSERT(reporter, GrResourceAllocator::AssignError::kNoError == error);
    REPORTER_ASSERT(reporter, 3 == startIndex && 4 == stopIndex);

    context->setResourceCacheLimit(origMaxBytes);
}

