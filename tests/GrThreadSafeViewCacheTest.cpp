/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkDeferredDisplayListRecorder.h"
#include "include/core/SkSurfaceCharacterization.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrThreadSafeUniquelyKeyedProxyViewCache.h"
#include "tests/Test.h"
#include "tests/TestUtils.h"

#include <thread>

static constexpr int kImageWH = 32;
static constexpr auto kImageOrigin = kBottomLeft_GrSurfaceOrigin;

static SkImageInfo default_ii(int wh) {
    return SkImageInfo::Make(wh, wh, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
}

static void create_key(GrUniqueKey* key, int wh) {
    static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
    GrUniqueKey::Builder builder(key, kDomain, 1);
    builder[0] = wh;
    builder.finish();
};

SkBitmap create_up_arrow_bitmap(int wh) {
    SkBitmap bitmap;

    bitmap.allocPixels(default_ii(wh));

    SkCanvas tmp(bitmap);
    tmp.clear(SK_ColorWHITE);

    SkPaint blue;
    blue.setColor(SK_ColorBLUE);
    blue.setAntiAlias(true);

    float halfW = wh / 2.0f;
    float halfH = wh / 2.0f;
    float thirdW = wh / 3.0f;

    SkPath path;
    path.moveTo(0, halfH);
    path.lineTo(thirdW, halfH);
    path.lineTo(thirdW, wh);
    path.lineTo(2*thirdW, wh);
    path.lineTo(2*thirdW, halfH);
    path.lineTo(wh, halfH);
    path.lineTo(halfW, 0);
    path.close();

    tmp.drawPath(path, blue);

    return bitmap;
}

class TestHelper {
public:
    struct Stats {
        int fCacheHits = 0;
        int fCacheMisses = 0;

        int fNumSWCreations = 0;
        int fNumHWCreations = 0;
    };

    TestHelper(GrDirectContext* dContext) : fDContext(dContext) {

        fDst = SkSurface::MakeRenderTarget(dContext, SkBudgeted::kNo, default_ii(kImageWH));
        SkAssertResult(fDst);

        SkSurfaceCharacterization characterization;
        SkAssertResult(fDst->characterize(&characterization));

        fRecorder1 = std::make_unique<SkDeferredDisplayListRecorder>(characterization);

        fRecorder2 = std::make_unique<SkDeferredDisplayListRecorder>(characterization);
    }

    ~TestHelper() {
        fDContext->flush();
        fDContext->submit(true);
    }

    Stats* stats() { return &fStats; }

    int numCacheEntries() const { return this->threadSafeViewCache()->numEntries(); }

    GrDirectContext* dContext() { return fDContext; }

    SkCanvas* liveCanvas() { return fDst ? fDst->getCanvas() : nullptr; }
    SkCanvas* ddlCanvas1() { return fRecorder1 ? fRecorder1->getCanvas() : nullptr; }
    sk_sp<SkDeferredDisplayList> snap1() {
        if (fRecorder1) {
            sk_sp<SkDeferredDisplayList> tmp = fRecorder1->detach();
            fRecorder1 = nullptr;
            return tmp;
        }

        return nullptr;
    }
    SkCanvas* ddlCanvas2() { return fRecorder2 ? fRecorder2->getCanvas() : nullptr; }
    sk_sp<SkDeferredDisplayList> snap2() {
        if (fRecorder2) {
            sk_sp<SkDeferredDisplayList> tmp = fRecorder2->detach();
            fRecorder2 = nullptr;
            return tmp;
        }

        return nullptr;
    }

    GrThreadSafeUniquelyKeyedProxyViewCache* threadSafeViewCache() {
        return fDContext->priv().threadSafeViewCache();
    }

    const GrThreadSafeUniquelyKeyedProxyViewCache* threadSafeViewCache() const {
        return fDContext->priv().threadSafeViewCache();
    }

    // Add a draw on 'canvas' that will introduce a ref on the 'wh' view
    void accessCachedView(SkCanvas* canvas,
                          int wh,
                          bool failLookup = false) {
        GrRecordingContext* rContext = canvas->recordingContext();

        auto view = AccessCachedView(rContext, this->threadSafeViewCache(),
                                     wh, failLookup, &fStats);
        SkASSERT(view);

        auto rtc = canvas->internal_private_accessTopLayerRenderTargetContext();

        rtc->drawTexture(nullptr,
                         view,
                         kPremul_SkAlphaType,
                         GrSamplerState::Filter::kNearest,
                         GrSamplerState::MipmapMode::kNone,
                         SkBlendMode::kSrcOver,
                         SkPMColor4f(),
                         SkRect::MakeWH(wh, wh),
                         SkRect::MakeWH(wh, wh),
                         GrAA::kNo,
                         GrQuadAAFlags::kNone,
                         SkCanvas::kFast_SrcRectConstraint,
                         SkMatrix::I(),
                         nullptr);
    }

    // Besides checking that the number of refs and cache hits and misses are as expected, this
    // method also validates that the unique key doesn't appear in any of the other caches.
    bool checkView(SkCanvas* canvas, int wh, int hits, int misses, int numRefs) {
        if (fStats.fCacheHits != hits || fStats.fCacheMisses != misses) {
            SkDebugf("Hits E: %d A: %d --- Misses E: %d A: %d\n",
                     hits, fStats.fCacheHits, misses, fStats.fCacheMisses);
            return false;
        }

        GrUniqueKey key;
        create_key(&key, wh);

        auto threadSafeViewCache = this->threadSafeViewCache();

        GrSurfaceProxyView view = threadSafeViewCache->find(key);
        if (!view.proxy()) {
            return false;
        }

        if (!view.proxy()->refCntGreaterThan(numRefs+1) ||  // +1 for 'view's ref
            view.proxy()->refCntGreaterThan(numRefs+2)) {
            return false;
        }

        if (canvas) {
            GrRecordingContext* rContext = canvas->recordingContext();
            GrProxyProvider* recordingProxyProvider = rContext->priv().proxyProvider();
            sk_sp<GrTextureProxy> result = recordingProxyProvider->findProxyByUniqueKey(key);
            if (result) {
                // views in this cache should never appear in the recorder's cache
                return false;
            }
        }

        {
            GrProxyProvider* directProxyProvider = fDContext->priv().proxyProvider();
            sk_sp<GrTextureProxy> result = directProxyProvider->findProxyByUniqueKey(key);
            if (result) {
                // views in this cache should never appear in the main proxy cache
                return false;
            }
        }

        {
            auto resourceProvider = fDContext->priv().resourceProvider();
            sk_sp<GrSurface> surf = resourceProvider->findByUniqueKey<GrSurface>(key);
            if (surf) {
                // the textures backing the views in this cache should never be discoverable in the
                // resource cache
                return false;
            }
        }

        return true;
    }

    size_t gpuSize(int wh) const {
        GrBackendFormat format = fDContext->defaultBackendFormat(kRGBA_8888_SkColorType,
                                                                 GrRenderable::kNo);

        return GrSurface::ComputeSize(*fDContext->priv().caps(), format,
                                      {wh, wh}, 1, GrMipMapped::kNo, false);
    }

private:
    static GrSurfaceProxyView AccessCachedView(GrRecordingContext*,
                                               GrThreadSafeUniquelyKeyedProxyViewCache*,
                                               int wh,
                                               bool failLookup, Stats*);
    static GrSurfaceProxyView CreateViewOnCpu(GrRecordingContext*, int wh, Stats*);
    static GrSurfaceProxyView CreateViewOnGpu(GrDirectContext*, int wh, Stats*);

    Stats fStats;
    GrDirectContext* fDContext = nullptr;

    sk_sp<SkSurface> fDst;
    std::unique_ptr<SkDeferredDisplayListRecorder> fRecorder1;
    std::unique_ptr<SkDeferredDisplayListRecorder> fRecorder2;
};

GrSurfaceProxyView TestHelper::CreateViewOnCpu(GrRecordingContext* rContext,
                                               int wh,
                                               Stats* stats) {
    GrProxyProvider* proxyProvider = rContext->priv().proxyProvider();

    sk_sp<GrTextureProxy> proxy = proxyProvider->createProxyFromBitmap(create_up_arrow_bitmap(wh),
                                                                       GrMipmapped::kNo,
                                                                       SkBackingFit::kExact,
                                                                       SkBudgeted::kYes);
    if (!proxy) {
        return {};
    }

    GrSwizzle swizzle = rContext->priv().caps()->getReadSwizzle(proxy->backendFormat(),
                                                                GrColorType::kRGBA_8888);
    ++stats->fNumSWCreations;
    return {std::move(proxy), kImageOrigin, swizzle};
}

GrSurfaceProxyView TestHelper::CreateViewOnGpu(GrDirectContext* dContext,
                                               int wh,
                                               Stats* stats) {
    GrProxyProvider* proxyProvider = dContext->priv().proxyProvider();

    sk_sp<GrTextureProxy> proxy = proxyProvider->createProxyFromBitmap(create_up_arrow_bitmap(wh),
                                                                       GrMipmapped::kNo,
                                                                       SkBackingFit::kExact,
                                                                       SkBudgeted::kYes);

    GrSwizzle swizzle = dContext->priv().caps()->getReadSwizzle(proxy->backendFormat(),
                                                                GrColorType::kRGBA_8888);
    ++stats->fNumHWCreations;
    return {std::move(proxy), kImageOrigin, swizzle};
}

// TODO: this doesn't actually implement the correct behavior for the gpu-thread. It needs to
// add a view to the cache and then queue up the calls to draw the content.
GrSurfaceProxyView TestHelper::AccessCachedView(
                                    GrRecordingContext* rContext,
                                    GrThreadSafeUniquelyKeyedProxyViewCache* threadSafeViewCache,
                                    int wh,
                                    bool failLookup,
                                    Stats* stats) {
    GrUniqueKey key;
    create_key(&key, wh);

    // We can "fail the lookup" to simulate a threaded race condition
    if (auto view = threadSafeViewCache->find(key); !failLookup && view) {
        ++stats->fCacheHits;
        return view;
    }

    ++stats->fCacheMisses;

    GrSurfaceProxyView view;
    if (GrDirectContext* dContext = rContext->asDirectContext()) {
        view = CreateViewOnGpu(dContext, wh, stats);
    } else {
        view = CreateViewOnCpu(rContext, wh, stats);
    }
    SkASSERT(view);

    return threadSafeViewCache->add(key, view);
}

// Case 1: ensure two DDL recorders share the view
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeViewCache1, reporter, ctxInfo) {
    TestHelper helper(ctxInfo.directContext());

    helper.accessCachedView(helper.ddlCanvas1(), kImageWH);
    REPORTER_ASSERT(reporter, helper.checkView(helper.ddlCanvas1(), kImageWH,
                                               /*hits*/ 0, /*misses*/ 1, /*refs*/ 1));

    helper.accessCachedView(helper.ddlCanvas2(), kImageWH);
    REPORTER_ASSERT(reporter, helper.checkView(helper.ddlCanvas2(), kImageWH,
                                               /*hits*/ 1, /*misses*/ 1, /*refs*/ 2));

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 1);
    REPORTER_ASSERT(reporter, helper.stats()->fNumHWCreations == 0);
    REPORTER_ASSERT(reporter, helper.stats()->fNumSWCreations == 1);
}

// Case 2: ensure that, if the direct context version wins, it is reused by the DDL recorders
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeViewCache2, reporter, ctxInfo) {
    TestHelper helper(ctxInfo.directContext());

    helper.accessCachedView(helper.liveCanvas(), kImageWH);
    REPORTER_ASSERT(reporter, helper.checkView(helper.liveCanvas(), kImageWH,
                                               /*hits*/ 0, /*misses*/ 1, /*refs*/ 1));

    helper.accessCachedView(helper.ddlCanvas1(), kImageWH);
    REPORTER_ASSERT(reporter, helper.checkView(helper.ddlCanvas1(), kImageWH,
                                               /*hits*/ 1, /*misses*/ 1, /*refs*/ 2));

    helper.accessCachedView(helper.ddlCanvas2(), kImageWH);
    REPORTER_ASSERT(reporter, helper.checkView(helper.ddlCanvas2(), kImageWH,
                                               /*hits*/ 2, /*misses*/ 1, /*refs*/ 3));

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 1);
    REPORTER_ASSERT(reporter, helper.stats()->fNumHWCreations == 1);
    REPORTER_ASSERT(reporter, helper.stats()->fNumSWCreations == 0);
}

// Case 3: ensure that, if the cpu-version wins, it is reused by the direct context
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeViewCache3, reporter, ctxInfo) {
    TestHelper helper(ctxInfo.directContext());

    helper.accessCachedView(helper.ddlCanvas1(), kImageWH);
    REPORTER_ASSERT(reporter, helper.checkView(helper.ddlCanvas1(), kImageWH,
                                               /*hits*/ 0, /*misses*/ 1, /*refs*/ 1));

    helper.accessCachedView(helper.liveCanvas(), kImageWH);
    REPORTER_ASSERT(reporter, helper.checkView(helper.liveCanvas(), kImageWH,
                                               /*hits*/ 1, /*misses*/ 1, /*refs*/ 2));

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 1);
    REPORTER_ASSERT(reporter, helper.stats()->fNumHWCreations == 0);
    REPORTER_ASSERT(reporter, helper.stats()->fNumSWCreations == 1);
}

// Case 4: ensure that, if two DDL recorders get in a race, they still end up sharing a single view
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeViewCache4, reporter, ctxInfo) {
    TestHelper helper(ctxInfo.directContext());

    helper.accessCachedView(helper.ddlCanvas1(), kImageWH);
    REPORTER_ASSERT(reporter, helper.checkView(helper.ddlCanvas1(), kImageWH,
                                               /*hits*/ 0, /*misses*/ 1, /*refs*/ 1));

    static const bool kFailLookup = true;
    helper.accessCachedView(helper.ddlCanvas2(), kImageWH, kFailLookup);
    REPORTER_ASSERT(reporter, helper.checkView(helper.ddlCanvas2(), kImageWH,
                                               /*hits*/ 0, /*misses*/ 2, /*refs*/ 2));

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 1);
    REPORTER_ASSERT(reporter, helper.stats()->fNumHWCreations == 0);
    REPORTER_ASSERT(reporter, helper.stats()->fNumSWCreations == 2);
}

// Case 5: ensure that expanding the map works
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeViewCache5, reporter, ctxInfo) {
    TestHelper helper(ctxInfo.directContext());

    auto threadSafeViewCache = helper.threadSafeViewCache();

    int size = 16;
    helper.accessCachedView(helper.ddlCanvas1(), size);

    size_t initialSize = threadSafeViewCache->approxBytesUsedForHash();

    while (initialSize == threadSafeViewCache->approxBytesUsedForHash()) {
        size *= 2;
        helper.accessCachedView(helper.ddlCanvas1(), size);
    }
}

// Case 6: check on dropping refs
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeViewCache6, reporter, ctxInfo) {
    TestHelper helper(ctxInfo.directContext());

    helper.accessCachedView(helper.ddlCanvas1(), kImageWH);
    sk_sp<SkDeferredDisplayList> ddl1 = helper.snap1();
    REPORTER_ASSERT(reporter, helper.checkView(nullptr, kImageWH,
                                               /*hits*/ 0, /*misses*/ 1, /*refs*/ 1));

    helper.accessCachedView(helper.ddlCanvas2(), kImageWH);
    sk_sp<SkDeferredDisplayList> ddl2 = helper.snap2();
    REPORTER_ASSERT(reporter, helper.checkView(nullptr, kImageWH,
                                               /*hits*/ 1, /*misses*/ 1, /*refs*/ 2));

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 1);

    ddl1 = nullptr;
    REPORTER_ASSERT(reporter, helper.checkView(nullptr, kImageWH,
                                               /*hits*/ 1, /*misses*/ 1, /*refs*/ 1));

    ddl2 = nullptr;
    REPORTER_ASSERT(reporter, helper.checkView(nullptr, kImageWH,
                                               /*hits*/ 1, /*misses*/ 1, /*refs*/ 0));

    // The cache still has its ref
    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 1);

    REPORTER_ASSERT(reporter, helper.checkView(nullptr, kImageWH,
                                               /*hits*/ 1, /*misses*/ 1, /*refs*/ 0));
}

// Case 7: check that invoking dropAllRefs and dropAllUniqueRefs directly works as expected
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeViewCache7, reporter, ctxInfo) {
    TestHelper helper(ctxInfo.directContext());

    helper.accessCachedView(helper.ddlCanvas1(), kImageWH);
    sk_sp<SkDeferredDisplayList> ddl1 = helper.snap1();
    REPORTER_ASSERT(reporter, helper.checkView(nullptr, kImageWH,
                                               /*hits*/ 0, /*misses*/ 1, /*refs*/ 1));

    helper.accessCachedView(helper.ddlCanvas2(), 2*kImageWH);
    sk_sp<SkDeferredDisplayList> ddl2 = helper.snap2();
    REPORTER_ASSERT(reporter, helper.checkView(nullptr, 2*kImageWH,
                                               /*hits*/ 0, /*misses*/ 2, /*refs*/ 1));

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 2);

    helper.threadSafeViewCache()->dropUniqueRefs(nullptr);
    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 2);

    ddl1 = nullptr;

    helper.threadSafeViewCache()->dropUniqueRefs(nullptr);
    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 1);
    REPORTER_ASSERT(reporter, helper.checkView(nullptr, 2*kImageWH,
                                               /*hits*/ 0, /*misses*/ 2, /*refs*/ 1));

    helper.threadSafeViewCache()->dropAllRefs();
    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 0);

    ddl2 = nullptr;
}

// Case 8: This checks that GrContext::abandonContext works as expected wrt the thread
//         safe cache. This simulates the case where we have one DDL that has finished
//         recording but one still recording when the abandonContext fires.
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeViewCache8, reporter, ctxInfo) {
    TestHelper helper(ctxInfo.directContext());

    helper.accessCachedView(helper.liveCanvas(), kImageWH);
    REPORTER_ASSERT(reporter, helper.checkView(helper.liveCanvas(), kImageWH,
                                               /*hits*/ 0, /*misses*/ 1, /*refs*/ 1));

    helper.accessCachedView(helper.ddlCanvas1(), kImageWH);
    sk_sp<SkDeferredDisplayList> ddl1 = helper.snap1();
    REPORTER_ASSERT(reporter, helper.checkView(helper.ddlCanvas1(), kImageWH,
                                               /*hits*/ 1, /*misses*/ 1, /*refs*/ 2));

    helper.accessCachedView(helper.ddlCanvas2(), kImageWH);
    REPORTER_ASSERT(reporter, helper.checkView(helper.ddlCanvas2(), kImageWH,
                                               /*hits*/ 2, /*misses*/ 1, /*refs*/ 3));

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 1);
    REPORTER_ASSERT(reporter, helper.stats()->fNumHWCreations == 1);
    REPORTER_ASSERT(reporter, helper.stats()->fNumSWCreations == 0);

    ctxInfo.directContext()->abandonContext(); // This should exercise dropAllRefs

    sk_sp<SkDeferredDisplayList> ddl2 = helper.snap2();

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 0);

    ddl1 = nullptr;
    ddl2 = nullptr;
}

// Case 9: This checks that GrContext::releaseResourcesAndAbandonContext works as expected wrt
//         the thread safe cache. This simulates the case where we have one DDL that has finished
//         recording but one still recording when the releaseResourcesAndAbandonContext fires.
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeViewCache9, reporter, ctxInfo) {
    TestHelper helper(ctxInfo.directContext());

    helper.accessCachedView(helper.liveCanvas(), kImageWH);
    REPORTER_ASSERT(reporter, helper.checkView(helper.liveCanvas(), kImageWH,
                                               /*hits*/ 0, /*misses*/ 1, /*refs*/ 1));

    helper.accessCachedView(helper.ddlCanvas1(), kImageWH);
    sk_sp<SkDeferredDisplayList> ddl1 = helper.snap1();
    REPORTER_ASSERT(reporter, helper.checkView(helper.ddlCanvas1(), kImageWH,
                                               /*hits*/ 1, /*misses*/ 1, /*refs*/ 2));

    helper.accessCachedView(helper.ddlCanvas2(), kImageWH);
    REPORTER_ASSERT(reporter, helper.checkView(helper.ddlCanvas2(), kImageWH,
                                               /*hits*/ 2, /*misses*/ 1, /*refs*/ 3));

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 1);
    REPORTER_ASSERT(reporter, helper.stats()->fNumHWCreations == 1);
    REPORTER_ASSERT(reporter, helper.stats()->fNumSWCreations == 0);

    ctxInfo.directContext()->releaseResourcesAndAbandonContext(); // This should hit dropAllRefs

    sk_sp<SkDeferredDisplayList> ddl2 = helper.snap2();

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 0);

    ddl1 = nullptr;
    ddl2 = nullptr;
}

// Case 10: This checks that the GrContext::purgeUnlockedResources(size_t) variant works as
//          expected wrt the thread safe cache. It, in particular, tests out the MRU behavior
//          of the shared cache.
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeViewCache10, reporter, ctxInfo) {
    auto dContext = ctxInfo.directContext();

    if (GrBackendApi::kOpenGL != dContext->backend()) {
        // The lower-level backends have too much going on for the following simple purging
        // test to work
        return;
    }

    TestHelper helper(dContext);

    helper.accessCachedView(helper.liveCanvas(), kImageWH);
    REPORTER_ASSERT(reporter, helper.checkView(helper.liveCanvas(), kImageWH,
                                               /*hits*/ 0, /*misses*/ 1, /*refs*/ 1));

    helper.accessCachedView(helper.ddlCanvas1(), kImageWH);
    sk_sp<SkDeferredDisplayList> ddl1 = helper.snap1();
    REPORTER_ASSERT(reporter, helper.checkView(helper.ddlCanvas1(), kImageWH,
                                               /*hits*/ 1, /*misses*/ 1, /*refs*/ 2));

    helper.accessCachedView(helper.liveCanvas(), 2*kImageWH);
    REPORTER_ASSERT(reporter, helper.checkView(helper.liveCanvas(), 2*kImageWH,
                                               /*hits*/ 1, /*misses*/ 2, /*refs*/ 1));

    helper.accessCachedView(helper.ddlCanvas2(), 2*kImageWH);
    sk_sp<SkDeferredDisplayList> ddl2 = helper.snap2();
    REPORTER_ASSERT(reporter, helper.checkView(helper.ddlCanvas2(), 2*kImageWH,
                                               /*hits*/ 2, /*misses*/ 2, /*refs*/ 2));

    dContext->flush();
    dContext->submit(true);

    // This should clear out everything but the textures locked in the thread-safe cache
    dContext->purgeUnlockedResources(false);

    ddl1 = nullptr;
    ddl2 = nullptr;

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 2);
    REPORTER_ASSERT(reporter, helper.checkView(helper.liveCanvas(), kImageWH,
                                               /*hits*/ 2, /*misses*/ 2, /*refs*/ 0));
    REPORTER_ASSERT(reporter, helper.checkView(helper.liveCanvas(), 2*kImageWH,
                                               /*hits*/ 2, /*misses*/ 2, /*refs*/ 0));

    // Regardless of which image is MRU, this should force the other out
    size_t desiredBytes = helper.gpuSize(2*kImageWH) + helper.gpuSize(kImageWH)/2;

    auto cache = dContext->priv().getResourceCache();
    size_t currentBytes = cache->getResourceBytes();

    SkASSERT(currentBytes >= desiredBytes);
    size_t amountToPurge = currentBytes - desiredBytes;

    // The 2*kImageWH texture should be MRU.
    dContext->purgeUnlockedResources(amountToPurge, true);

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 1);

    REPORTER_ASSERT(reporter, helper.checkView(helper.liveCanvas(), 2*kImageWH,
                                               /*hits*/ 2, /*misses*/ 2, /*refs*/ 0));
}

// Case 11: This checks that scratch-only variant of GrContext::purgeUnlockedResources works as
//          expected wrt the thread safe cache.
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeViewCache11, reporter, ctxInfo) {
    auto dContext = ctxInfo.directContext();

    TestHelper helper(dContext);

    helper.accessCachedView(helper.liveCanvas(), kImageWH);
    REPORTER_ASSERT(reporter, helper.checkView(helper.liveCanvas(), kImageWH,
                                               /*hits*/ 0, /*misses*/ 1, /*refs*/ 1));

    helper.accessCachedView(helper.liveCanvas(), 2*kImageWH);
    REPORTER_ASSERT(reporter, helper.checkView(helper.liveCanvas(), 2*kImageWH,
                                               /*hits*/ 0, /*misses*/ 2, /*refs*/ 1));

    dContext->flush();
    dContext->submit(true);

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 2);
    REPORTER_ASSERT(reporter, helper.checkView(helper.liveCanvas(), kImageWH,
                                               /*hits*/ 0, /*misses*/ 2, /*refs*/ 0));
    REPORTER_ASSERT(reporter, helper.checkView(helper.liveCanvas(), 2*kImageWH,
                                               /*hits*/ 0, /*misses*/ 2, /*refs*/ 0));

    // This shouldn't remove anything from the cache
    dContext->purgeUnlockedResources(/* scratchResourcesOnly */ true);

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 2);

    dContext->purgeUnlockedResources(/* scratchResourcesOnly */ false);

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 0);
}

// Case 12: Test out purges caused by resetting the cache budget
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeViewCache12, reporter, ctxInfo) {
    auto dContext = ctxInfo.directContext();

    TestHelper helper(dContext);

    helper.accessCachedView(helper.liveCanvas(), kImageWH);
    REPORTER_ASSERT(reporter, helper.checkView(helper.liveCanvas(), kImageWH,
                                               /*hits*/ 0, /*misses*/ 1, /*refs*/ 1));
    helper.accessCachedView(helper.ddlCanvas1(), kImageWH);
    sk_sp<SkDeferredDisplayList> ddl1 = helper.snap1();
    REPORTER_ASSERT(reporter, helper.checkView(helper.ddlCanvas1(), kImageWH,
                                               /*hits*/ 1, /*misses*/ 1, /*refs*/ 2));

    helper.accessCachedView(helper.liveCanvas(), 2*kImageWH);
    REPORTER_ASSERT(reporter, helper.checkView(helper.liveCanvas(), 2*kImageWH,
                                               /*hits*/ 1, /*misses*/ 2, /*refs*/ 1));

    dContext->flush();
    dContext->submit(true);

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 2);
    REPORTER_ASSERT(reporter, helper.checkView(helper.liveCanvas(), kImageWH,
                                               /*hits*/ 1, /*misses*/ 2, /*refs*/ 1));
    REPORTER_ASSERT(reporter, helper.checkView(helper.liveCanvas(), 2*kImageWH,
                                               /*hits*/ 1, /*misses*/ 2, /*refs*/ 0));

    dContext->setResourceCacheLimit(0);

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 1);

    ddl1 = nullptr;

    dContext->performDeferredCleanup(std::chrono::milliseconds(0));

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 0);
}

// Case 13: Test out the 'msNotUsed' parameter to GrContext::performDeferredCleanup.
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeViewCache13, reporter, ctxInfo) {
    auto dContext = ctxInfo.directContext();

    TestHelper helper(dContext);

    helper.accessCachedView(helper.ddlCanvas1(), kImageWH);

    REPORTER_ASSERT(reporter, helper.checkView(helper.ddlCanvas1(), kImageWH,
                                               /*hits*/ 0, /*misses*/ 1, /*refs*/ 1));
    sk_sp<SkDeferredDisplayList> ddl1 = helper.snap1();

    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    auto firstTime = GrStdSteadyClock::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    helper.accessCachedView(helper.ddlCanvas2(), 2*kImageWH);
    REPORTER_ASSERT(reporter, helper.checkView(helper.ddlCanvas2(), 2*kImageWH,
                                               /*hits*/ 0, /*misses*/ 2, /*refs*/ 1));
    sk_sp<SkDeferredDisplayList> ddl2 = helper.snap2();

    ddl1 = nullptr;
    ddl2 = nullptr;

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 2);

    auto secondTime = GrStdSteadyClock::now();

    auto msecs = std::chrono::duration_cast<std::chrono::milliseconds>(secondTime - firstTime);
    dContext->performDeferredCleanup(msecs);

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 1);
    REPORTER_ASSERT(reporter, helper.checkView(helper.liveCanvas(), 2*kImageWH,
                                               /*hits*/ 0, /*misses*/ 2, /*refs*/ 0));
}
