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

        SkBitmap tmp = create_up_arrow_bitmap(kImageWH);
        SkAssertResult(CreateBackendTexture(fDContext, &fBETex, tmp));
    }

    ~TestHelper() {
        DeleteBackendTexture(fDContext, fBETex);
    }

    Stats* stats() { return &fStats; }

    int numCacheEntries() const { return this->threadSafeViewCache()->numEntries(); }

    GrDirectContext* dContext() { return fDContext; }

    SkCanvas* liveCanvas() { return fDst ? fDst->getCanvas() : nullptr; }
    SkCanvas* ddlCanvas1() { return fRecorder1 ? fRecorder1->getCanvas() : nullptr; }
    SkCanvas* ddlCanvas2() { return fRecorder2 ? fRecorder2->getCanvas() : nullptr; }

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
                                     fBETex, wh, failLookup, &fStats);
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

    // TODO: make a static version and pass in rContext & resourceCache
    bool checkView(SkCanvas* canvas, int wh, int hits, int misses, int numRefs) {
        if (fStats.fCacheHits != hits || fStats.fCacheMisses != misses) {
            return false;
        }

        GrUniqueKey key;
        create_key(&key, wh);

        GrRecordingContext* rContext = canvas->recordingContext();
        auto threadSafeViewCache = this->threadSafeViewCache();

        GrSurfaceProxyView view = threadSafeViewCache->find(key);

        if (!view.proxy()->refCntGreaterThan(numRefs+1) ||  // +1 for 'view's ref
            view.proxy()->refCntGreaterThan(numRefs+2)) {
            return false;
        }

        {
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

private:
    static GrSurfaceProxyView AccessCachedView(GrRecordingContext*,
                                               GrThreadSafeUniquelyKeyedProxyViewCache*,
                                               GrBackendTexture, int wh,
                                               bool failLookup, Stats*);
    static GrSurfaceProxyView CreateViewOnCpu(GrRecordingContext*, int wh, Stats*);
    static GrSurfaceProxyView CreateViewOnGpu(GrDirectContext*, GrBackendTexture, int wh, Stats*);

    Stats fStats;
    GrDirectContext* fDContext = nullptr;

    sk_sp<SkSurface> fDst;
    std::unique_ptr<SkDeferredDisplayListRecorder> fRecorder1;
    std::unique_ptr<SkDeferredDisplayListRecorder> fRecorder2;

    GrBackendTexture fBETex;
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
                                               GrBackendTexture beTex,
                                               int wh,
                                               Stats* stats) {
    SkASSERT(!stats->fNumHWCreations); // we had better not do this more than once

    GrProxyProvider* proxyProvider = dContext->priv().proxyProvider();

    sk_sp<GrTextureProxy> proxy = proxyProvider->wrapBackendTexture(beTex,
                                                                    kBorrow_GrWrapOwnership,
                                                                    GrWrapCacheable::kNo,
                                                                    kRead_GrIOType);
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
                                    GrBackendTexture beTex,
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
        view = CreateViewOnGpu(dContext, beTex, wh, stats);
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
    helper.checkView(helper.ddlCanvas1(), kImageWH, /*hits*/ 0, /*misses*/ 1, /*refs*/ 1);

    helper.accessCachedView(helper.ddlCanvas2(), kImageWH);
    helper.checkView(helper.ddlCanvas2(), kImageWH, /*hits*/ 1, /*misses*/ 1, /*refs*/ 2);

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 1);
    REPORTER_ASSERT(reporter, helper.stats()->fNumHWCreations == 0);
    REPORTER_ASSERT(reporter, helper.stats()->fNumSWCreations == 1);
}

// Case 2: ensure that, if the direct context version wins, it is reused by the DDL recorders
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeViewCache2, reporter, ctxInfo) {
    TestHelper helper(ctxInfo.directContext());

    helper.accessCachedView(helper.liveCanvas(), kImageWH);
    helper.checkView(helper.liveCanvas(), kImageWH, /*hits*/ 0, /*misses*/ 1, /*refs*/ 1);

    helper.accessCachedView(helper.ddlCanvas1(), kImageWH);
    helper.checkView(helper.ddlCanvas1(), kImageWH, /*hits*/ 1, /*misses*/ 1, /*refs*/ 2);

    helper.accessCachedView(helper.ddlCanvas2(), kImageWH);
    helper.checkView(helper.ddlCanvas2(), kImageWH, /*hits*/ 2, /*misses*/ 1, /*refs*/ 3);

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 1);
    REPORTER_ASSERT(reporter, helper.stats()->fNumHWCreations == 1);
    REPORTER_ASSERT(reporter, helper.stats()->fNumSWCreations == 0);
}

// Case 3: ensure that, if the cpu-version wins, it is reused by the direct context
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeViewCache3, reporter, ctxInfo) {
    TestHelper helper(ctxInfo.directContext());

    helper.accessCachedView(helper.ddlCanvas1(), kImageWH);
    helper.checkView(helper.ddlCanvas1(), kImageWH, /*hits*/ 0, /*misses*/ 1, /*refs*/ 1);

    helper.accessCachedView(helper.liveCanvas(), kImageWH);
    helper.checkView(helper.liveCanvas(), kImageWH, /*hits*/ 1, /*misses*/ 1, /*refs*/ 2);

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 1);
    REPORTER_ASSERT(reporter, helper.stats()->fNumHWCreations == 0);
    REPORTER_ASSERT(reporter, helper.stats()->fNumSWCreations == 1);
}

// Case 4: ensure that, if two DDL recorders get in a race, they still end up sharing a single view
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeViewCache4, reporter, ctxInfo) {
    TestHelper helper(ctxInfo.directContext());

    helper.accessCachedView(helper.ddlCanvas1(), kImageWH);
    helper.checkView(helper.ddlCanvas1(), kImageWH, /*hits*/ 0, /*misses*/ 1, /*refs*/ 1);

    static const bool kFailLookup = true;
    helper.accessCachedView(helper.ddlCanvas2(), kImageWH, kFailLookup);
    helper.checkView(helper.ddlCanvas2(), kImageWH, /*hits*/ 0, /*misses*/ 2, /*refs*/ 2);

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

    int initialCount = threadSafeViewCache->count();

    while (initialCount == threadSafeViewCache->count()) {
        size *= 2;
        helper.accessCachedView(helper.ddlCanvas1(), size);
    }
}

