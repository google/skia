/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTo.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ResourceKey.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/GrResourceAllocator.h"
#include "src/gpu/ganesh/GrResourceCache.h"
#include "src/gpu/ganesh/GrResourceProvider.h"
#include "src/gpu/ganesh/GrSurface.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyPriv.h"
#include "src/gpu/ganesh/GrTexture.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/gpu/ManagedBackendTexture.h"

#include <array>
#include <cstddef>
#include <functional>
#include <utility>

using namespace skia_private;

class GrRecordingContext;
struct GrContextOptions;

namespace {
struct ProxyParams {
    int             fSize;
    GrRenderable    fRenderable;
    GrColorType     fColorType;
    SkBackingFit    fFit;
    int             fSampleCnt;
    skgpu::Budgeted fBudgeted;
    enum Kind {
        kDeferred,
        kBackend,
        kFullyLazy,
        kLazy,
        kInstantiated
    };
    Kind            fKind;
    skgpu::UniqueKey     fUniqueKey = skgpu::UniqueKey();
    // TODO: do we care about mipmapping
};

constexpr GrRenderable kRT = GrRenderable::kYes;
constexpr GrRenderable kNotRT = GrRenderable::kNo;

constexpr GrColorType kRGBA = GrColorType::kRGBA_8888;
constexpr GrColorType kAlpha = GrColorType::kAlpha_8;

constexpr SkBackingFit kE = SkBackingFit::kExact;
constexpr SkBackingFit kA = SkBackingFit::kApprox;

constexpr skgpu::Budgeted kNotB = skgpu::Budgeted::kNo;
constexpr skgpu::Budgeted kB = skgpu::Budgeted::kYes;

constexpr ProxyParams::Kind kDeferred = ProxyParams::Kind::kDeferred;
constexpr ProxyParams::Kind kBackend = ProxyParams::Kind::kBackend;
constexpr ProxyParams::Kind kInstantiated = ProxyParams::Kind::kInstantiated;
constexpr ProxyParams::Kind kLazy = ProxyParams::Kind::kLazy;
constexpr ProxyParams::Kind kFullyLazy = ProxyParams::Kind::kFullyLazy;
}

static sk_sp<GrSurfaceProxy> make_deferred(GrProxyProvider* proxyProvider, const GrCaps* caps,
                                           const ProxyParams& p) {
    const GrBackendFormat format = caps->getDefaultBackendFormat(p.fColorType, p.fRenderable);
    return proxyProvider->createProxy(format,
                                      {p.fSize, p.fSize},
                                      p.fRenderable,
                                      p.fSampleCnt,
                                      skgpu::Mipmapped::kNo,
                                      p.fFit,
                                      p.fBudgeted,
                                      GrProtected::kNo,
                                      /*label=*/"ResourceAllocatorTest_Deffered");
}

static sk_sp<GrSurfaceProxy> make_backend(GrDirectContext* dContext, const ProxyParams& p) {
    GrProxyProvider* proxyProvider = dContext->priv().proxyProvider();

    SkColorType skColorType = GrColorTypeToSkColorType(p.fColorType);
    SkASSERT(SkColorType::kUnknown_SkColorType != skColorType);

    auto mbet = sk_gpu_test::ManagedBackendTexture::MakeWithoutData(
            dContext, p.fSize, p.fSize, skColorType, skgpu::Mipmapped::kNo, GrRenderable::kNo);

    if (!mbet) {
        return nullptr;
    }

    return proxyProvider->wrapBackendTexture(mbet->texture(),
                                             kBorrow_GrWrapOwnership,
                                             GrWrapCacheable::kNo,
                                             kRead_GrIOType,
                                             mbet->refCountedCallback());
}

static sk_sp<GrSurfaceProxy> make_fully_lazy(GrProxyProvider* proxyProvider, const GrCaps* caps,
                                             const ProxyParams& p) {
    const GrBackendFormat format = caps->getDefaultBackendFormat(p.fColorType, p.fRenderable);
    auto cb = [p](GrResourceProvider* provider, const GrSurfaceProxy::LazySurfaceDesc& desc) {
        auto tex = provider->createTexture({p.fSize, p.fSize},
                                           desc.fFormat,
                                           desc.fTextureType,
                                           desc.fRenderable,
                                           desc.fSampleCnt,
                                           desc.fMipmapped,
                                           desc.fBudgeted,
                                           desc.fProtected,
                                           /*label=*/"ResourceAllocatorTest_FullLazy");
        return GrSurfaceProxy::LazyCallbackResult(std::move(tex));
    };
    return GrProxyProvider::MakeFullyLazyProxy(std::move(cb), format, p.fRenderable, p.fSampleCnt,
                                               GrProtected::kNo, *caps,
                                               GrSurfaceProxy::UseAllocator::kYes);
}

static sk_sp<GrSurfaceProxy> make_lazy(GrProxyProvider* proxyProvider, const GrCaps* caps,
                                       const ProxyParams& p) {
    const GrBackendFormat format = caps->getDefaultBackendFormat(p.fColorType, p.fRenderable);
    auto cb = [](GrResourceProvider* provider, const GrSurfaceProxy::LazySurfaceDesc& desc) {
        auto tex = provider->createTexture(desc.fDimensions,
                                           desc.fFormat,
                                           desc.fTextureType,
                                           desc.fRenderable,
                                           desc.fSampleCnt,
                                           desc.fMipmapped,
                                           desc.fBudgeted,
                                           desc.fProtected,
                                           /*label=*/"ResourceAllocatorTest_Lazy");
        return GrSurfaceProxy::LazyCallbackResult(std::move(tex));
    };
    return proxyProvider->createLazyProxy(std::move(cb),
                                          format,
                                          {p.fSize, p.fSize},
                                          skgpu::Mipmapped::kNo,
                                          GrMipmapStatus::kNotAllocated,
                                          GrInternalSurfaceFlags::kNone,
                                          p.fFit,
                                          p.fBudgeted,
                                          GrProtected::kNo,
                                          GrSurfaceProxy::UseAllocator::kYes,
                                          /*label=*/{});
}

static sk_sp<GrSurfaceProxy> make_proxy(GrDirectContext* dContext, const ProxyParams& p) {
    GrProxyProvider* proxyProvider = dContext->priv().proxyProvider();
    const GrCaps* caps = dContext->priv().caps();
    sk_sp<GrSurfaceProxy> proxy;
    switch (p.fKind) {
        case ProxyParams::kDeferred:
            proxy = make_deferred(proxyProvider, caps, p);
            break;
        case ProxyParams::kBackend:
            proxy = make_backend(dContext, p);
            break;
        case ProxyParams::kFullyLazy:
            proxy = make_fully_lazy(proxyProvider, caps, p);
            break;
        case ProxyParams::kLazy:
            proxy = make_lazy(proxyProvider, caps, p);
            break;
        case ProxyParams::kInstantiated:
            proxy = make_deferred(proxyProvider, caps, p);
            if (proxy) {
                auto surf = proxy->priv().createSurface(dContext->priv().resourceProvider());
                proxy->priv().assign(std::move(surf));
            }
            break;
    }
    if (proxy && p.fUniqueKey.isValid()) {
        SkASSERT(proxy->asTextureProxy());
        proxyProvider->assignUniqueKeyToProxy(p.fUniqueKey, proxy->asTextureProxy());
    }
    return proxy;
}

// Basic test that two proxies with overlapping intervals and compatible descriptors are
// assigned different GrSurfaces.
static void overlap_test(skiatest::Reporter* reporter, GrDirectContext* dContext,
                         sk_sp<GrSurfaceProxy> p1, sk_sp<GrSurfaceProxy> p2,
                         bool expectedResult) {
    GrResourceAllocator alloc(dContext);

    alloc.addInterval(p1.get(), 0, 4, GrResourceAllocator::ActualUse::kYes);
    alloc.incOps();
    alloc.addInterval(p2.get(), 1, 2, GrResourceAllocator::ActualUse::kYes);
    alloc.incOps();

    REPORTER_ASSERT(reporter, alloc.planAssignment());
    REPORTER_ASSERT(reporter, alloc.makeBudgetHeadroom());
    REPORTER_ASSERT(reporter, alloc.assign());

    REPORTER_ASSERT(reporter, p1->peekSurface());
    REPORTER_ASSERT(reporter, p2->peekSurface());
    bool doTheBackingStoresMatch = p1->underlyingUniqueID() == p2->underlyingUniqueID();
    REPORTER_ASSERT(reporter, expectedResult == doTheBackingStoresMatch);
}

// Test various cases when two proxies do not have overlapping intervals.
// This mainly acts as a test of the ResourceAllocator's free pool.
static void non_overlap_test(skiatest::Reporter* reporter, GrDirectContext* dContext,
                             sk_sp<GrSurfaceProxy> p1, sk_sp<GrSurfaceProxy> p2,
                             bool expectedResult) {
    GrResourceAllocator alloc(dContext);

    alloc.incOps();
    alloc.incOps();
    alloc.incOps();
    alloc.incOps();
    alloc.incOps();
    alloc.incOps();

    alloc.addInterval(p1.get(), 0, 2, GrResourceAllocator::ActualUse::kYes);
    alloc.addInterval(p2.get(), 3, 5, GrResourceAllocator::ActualUse::kYes);

    REPORTER_ASSERT(reporter, alloc.planAssignment());
    REPORTER_ASSERT(reporter, alloc.makeBudgetHeadroom());
    REPORTER_ASSERT(reporter, alloc.assign());

    REPORTER_ASSERT(reporter, p1->peekSurface());
    REPORTER_ASSERT(reporter, p2->peekSurface());
    bool doTheBackingStoresMatch = p1->underlyingUniqueID() == p2->underlyingUniqueID();
    REPORTER_ASSERT(reporter, expectedResult == doTheBackingStoresMatch);
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(ResourceAllocatorTest,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    auto dContext = ctxInfo.directContext();
    const GrCaps* caps = dContext->priv().caps();

    struct TestCase {
        ProxyParams   fP1;
        ProxyParams   fP2;
        bool          fExpectation;
    };

    constexpr bool kShare = true;
    constexpr bool kDontShare = false;

    // Non-RT GrSurfaces are never recycled on some platforms.
    bool kConditionallyShare = caps->reuseScratchTextures();

    static const TestCase overlappingTests[] = {
        // Two proxies with overlapping intervals and compatible descriptors should never share
        // RT version
        {{64, kRT, kRGBA, kA, 1, kNotB, kDeferred},
         {64, kRT, kRGBA, kA, 1, kNotB, kDeferred},
         kDontShare},
        // non-RT version
        {{64, kNotRT, kRGBA, kA, 1, kNotB, kDeferred},
         {64, kNotRT, kRGBA, kA, 1, kNotB, kDeferred},
         kDontShare},
    };

    for (size_t i = 0; i < std::size(overlappingTests); i++) {
        const TestCase& test = overlappingTests[i];
        sk_sp<GrSurfaceProxy> p1 = make_proxy(dContext, test.fP1);
        sk_sp<GrSurfaceProxy> p2 = make_proxy(dContext, test.fP2);
        reporter->push(SkStringPrintf("case %d", SkToInt(i)));
        overlap_test(reporter, dContext, std::move(p1), std::move(p2), test.fExpectation);
        reporter->pop();
    }

    auto beFormat = caps->getDefaultBackendFormat(GrColorType::kRGBA_8888, GrRenderable::kYes);
    int k2 = caps->getRenderTargetSampleCount(2, beFormat);
    int k4 = caps->getRenderTargetSampleCount(4, beFormat);

    // This cannot be made static as some of the members depend on non static variables like
    // kConditionallyShare, k2, and k4.
    const TestCase nonOverlappingTests[] = {
        // Two non-overlapping intervals w/ compatible proxies should share
        // both same size & approx
        {{64, kRT, kRGBA, kA, 1, kNotB, kDeferred},
         {64, kRT, kRGBA, kA, 1, kNotB, kDeferred},
         kShare},
        {{64, kNotRT, kRGBA, kA, 1, kNotB, kDeferred},
         {64, kNotRT, kRGBA, kA, 1, kNotB, kDeferred},
         kConditionallyShare},
        // diffs sizes but still approx
        {{64, kRT, kRGBA, kA, 1, kNotB, kDeferred},
         {50, kRT, kRGBA, kA, 1, kNotB, kDeferred},
         kShare},
        {{64, kNotRT, kRGBA, kA, 1, kNotB, kDeferred},
         {50, kNotRT, kRGBA, kA, 1, kNotB, kDeferred},
         kConditionallyShare},
        // sames sizes but exact
        {{64, kRT, kRGBA, kE, 1, kNotB, kDeferred},
         {64, kRT, kRGBA, kE, 1, kNotB, kDeferred},
         kShare},
        {{64, kNotRT, kRGBA, kE, 1, kNotB, kDeferred},
         {64, kNotRT, kRGBA, kE, 1, kNotB, kDeferred},
         kConditionallyShare},
        // Two non-overlapping intervals w/ different exact sizes should not share
        {{56, kRT, kRGBA, kE, 1, kNotB, kDeferred},
         {54, kRT, kRGBA, kE, 1, kNotB, kDeferred},
         kDontShare},
        // Two non-overlapping intervals w/ _very different_ approx sizes should not share
        {{255, kRT, kRGBA, kA, 1, kNotB, kDeferred},
         {127, kRT, kRGBA, kA, 1, kNotB, kDeferred},
         kDontShare},
        // Two non-overlapping intervals w/ different MSAA sample counts should not share
        {{64, kRT, kRGBA, kA, k2, kNotB, kDeferred},
         {64, kRT, kRGBA, kA, k4, kNotB, kDeferred},
         k2 == k4},
        // Two non-overlapping intervals w/ different configs should not share
        {{64, kRT, kRGBA, kA, 1, kNotB, kDeferred},
         {64, kRT, kAlpha, kA, 1, kNotB, kDeferred},
         kDontShare},
        // Two non-overlapping intervals w/ different RT classifications should never share
        {{64, kRT, kRGBA, kA, 1, kNotB, kDeferred},
         {64, kNotRT, kRGBA, kA, 1, kNotB, kDeferred},
         kDontShare},
        {{64, kNotRT, kRGBA, kA, 1, kNotB, kDeferred},
         {64, kRT, kRGBA, kA, 1, kNotB, kDeferred},
         kDontShare},
        // Two non-overlapping intervals w/ different origins should share
        {{64, kRT, kRGBA, kA, 1, kNotB, kDeferred},
         {64, kRT, kRGBA, kA, 1, kNotB, kDeferred},
         kShare},
        // Wrapped backend textures should never be reused
        {{64, kNotRT, kRGBA, kE, 1, kNotB, kBackend},
         {64, kNotRT, kRGBA, kE, 1, kNotB, kDeferred},
         kDontShare}
    };

    for (size_t i = 0; i < std::size(nonOverlappingTests); i++) {
        const TestCase& test = nonOverlappingTests[i];
        sk_sp<GrSurfaceProxy> p1 = make_proxy(dContext, test.fP1);
        sk_sp<GrSurfaceProxy> p2 = make_proxy(dContext, test.fP2);

        if (!p1 || !p2) {
            continue; // creation can fail (e.g., for msaa4 on iOS)
        }

        reporter->push(SkStringPrintf("case %d", SkToInt(i)));
        non_overlap_test(reporter, dContext, std::move(p1), std::move(p2),
                         test.fExpectation);
        reporter->pop();
    }
}

static void draw(GrRecordingContext* rContext) {
    SkImageInfo ii = SkImageInfo::Make(1024, 1024, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    sk_sp<SkSurface> s = SkSurfaces::RenderTarget(
            rContext, skgpu::Budgeted::kYes, ii, 1, kTopLeft_GrSurfaceOrigin, nullptr);

    SkCanvas* c = s->getCanvas();

    c->clear(SK_ColorBLACK);
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(ResourceAllocatorStressTest,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
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

struct Interval {
    ProxyParams           fParams;
    int                   fStart;
    int                   fEnd;
    sk_sp<GrSurfaceProxy> fProxy = nullptr;
};

struct TestCase {
    const char *          fName;
    bool                  fShouldFit;
    size_t                fBudget;
    TArray<ProxyParams> fPurgeableResourcesInCache = {};
    TArray<ProxyParams> fUnpurgeableResourcesInCache = {};
    TArray<Interval>    fIntervals;
};

static void memory_budget_test(skiatest::Reporter* reporter,
                               GrDirectContext* dContext,
                               const TestCase& test) {
    // Reset cache.
    auto cache = dContext->priv().getResourceCache();
    cache->releaseAll();
    cache->setLimit(test.fBudget);

    // Add purgeable entries.
    size_t expectedPurgeableBytes = 0;
    TArray<sk_sp<GrSurface>> purgeableSurfaces;
    for (auto& params : test.fPurgeableResourcesInCache) {
        SkASSERT(params.fKind == kInstantiated);
        sk_sp<GrSurfaceProxy> proxy = make_proxy(dContext, params);
        REPORTER_ASSERT(reporter, proxy->peekSurface());
        expectedPurgeableBytes += proxy->gpuMemorySize();
        purgeableSurfaces.push_back(sk_ref_sp(proxy->peekSurface()));
    }
    purgeableSurfaces.clear();
    REPORTER_ASSERT(reporter, expectedPurgeableBytes == cache->getPurgeableBytes(),
                    "%zu", cache->getPurgeableBytes());

    // Add unpurgeable entries.
    size_t expectedUnpurgeableBytes = 0;
    TArray<sk_sp<GrSurface>> unpurgeableSurfaces;
    for (auto& params : test.fUnpurgeableResourcesInCache) {
        SkASSERT(params.fKind == kInstantiated);
        sk_sp<GrSurfaceProxy> proxy = make_proxy(dContext, params);
        REPORTER_ASSERT(reporter, proxy->peekSurface());
        expectedUnpurgeableBytes += proxy->gpuMemorySize();
        unpurgeableSurfaces.push_back(sk_ref_sp(proxy->peekSurface()));
    }

    auto unpurgeableBytes = cache->getBudgetedResourceBytes() - cache->getPurgeableBytes();
    REPORTER_ASSERT(reporter, expectedUnpurgeableBytes == unpurgeableBytes,
                    "%zu", unpurgeableBytes);

    // Add intervals and test.
    GrResourceAllocator alloc(dContext);
    for (auto& interval : test.fIntervals) {
        for (int i = interval.fStart; i <= interval.fEnd; i++) {
            alloc.incOps();
        }
        alloc.addInterval(interval.fProxy.get(), interval.fStart, interval.fEnd,
                          GrResourceAllocator::ActualUse::kYes);
    }
    REPORTER_ASSERT(reporter, alloc.planAssignment());
    REPORTER_ASSERT(reporter, alloc.makeBudgetHeadroom() == test.fShouldFit);
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(ResourceAllocatorMemoryBudgetTest,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    auto dContext = ctxInfo.directContext();

    constexpr bool    kUnder               = true;
    constexpr bool    kOver                = false;
    constexpr size_t  kRGBA64Bytes         = 4 * 64 * 64;
    const ProxyParams kProxy64             = {64, kRT, kRGBA, kE, 1, kB,    kDeferred};
    const ProxyParams kProxy64NotBudgeted  = {64, kRT, kRGBA, kE, 1, kNotB, kDeferred};
    const ProxyParams kProxy64Lazy         = {64, kRT, kRGBA, kE, 1, kB,    kLazy};
    const ProxyParams kProxy64FullyLazy    = {64, kRT, kRGBA, kE, 1, kB,    kFullyLazy};
    const ProxyParams kProxy32Instantiated = {32, kRT, kRGBA, kE, 1, kB,    kInstantiated};
    const ProxyParams kProxy64Instantiated = {64, kRT, kRGBA, kE, 1, kB,    kInstantiated};

    TestCase tests[] = {
        {"empty DAG", kUnder, 0, {}, {}, {}},
        {"unbudgeted", kUnder, 0, {}, {}, {{kProxy64NotBudgeted, 0, 2}}},
        {"basic", kUnder, kRGBA64Bytes, {}, {}, {{kProxy64, 0, 2}}},
        {"basic, over", kOver, kRGBA64Bytes - 1, {}, {}, {{kProxy64, 0, 2}}},
        {"shared", kUnder, kRGBA64Bytes, {}, {},
            {
                {kProxy64, 0, 2},
                {kProxy64, 3, 5},
            }},
        {"retrieved from cache", kUnder, kRGBA64Bytes,
            /* purgeable */{kProxy64Instantiated},
            /* unpurgeable */{},
            {
                {kProxy64, 0, 2}
            }},
        {"purge 4", kUnder, kRGBA64Bytes,
            /* purgeable */{
                kProxy32Instantiated,
                kProxy32Instantiated,
                kProxy32Instantiated,
                kProxy32Instantiated
            },
            /* unpurgeable */{},
            {
                {kProxy64, 0, 2}
            }},
        {"dont purge what we've reserved", kOver, kRGBA64Bytes,
            /* purgeable */{kProxy64Instantiated},
            /* unpurgeable */{},
            {
                {kProxy64, 0, 2},
                {kProxy64, 1, 3}
            }},
        {"unpurgeable", kOver, kRGBA64Bytes,
            /* purgeable */{},
            /* unpurgeable */{kProxy64Instantiated},
            {
                {kProxy64, 0, 2}
            }},
        {"lazy", kUnder, kRGBA64Bytes,
            /* purgeable */{},
            /* unpurgeable */{},
            {
                {kProxy64Lazy, 0, 2}
            }},
        {"lazy, over", kOver, kRGBA64Bytes - 1,
            /* purgeable */{},
            /* unpurgeable */{},
            {
                {kProxy64Lazy, 0, 2}
            }},
        {"fully-lazy", kUnder, kRGBA64Bytes,
            /* purgeable */{},
            /* unpurgeable */{},
            {
                {kProxy64FullyLazy, 0, 2}
            }},
        {"fully-lazy, over", kOver, kRGBA64Bytes - 1,
            /* purgeable */{},
            /* unpurgeable */{},
            {
                {kProxy64FullyLazy, 0, 2}
            }},
    };
    SkString match("");
    for (size_t i = 0; i < std::size(tests); i++) {
        TestCase& test = tests[i];
        if (match.isEmpty() || match == SkString(test.fName)) {
            // Create proxies
            for (Interval& interval : test.fIntervals) {
                interval.fProxy = make_proxy(dContext, interval.fParams);
            }
            reporter->push(SkString(test.fName));
            memory_budget_test(reporter, dContext, test);
            reporter->pop();
        }
    }
}
