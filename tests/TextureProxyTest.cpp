/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test.

#include "Test.h"

#if SK_SUPPORT_GPU

#include "GrResourceCache.h"
#include "GrResourceProvider.h"
#include "GrTexture.h"
#include "GrTextureProxy.h"
#include "GrTextureProxyPriv.h"

#include "SkGr.h"
#include "SkImage.h"

int GrResourceCache::numUniqueKeyProxies_TestOnly() const {
    return fUniquelyKeyedProxies.count();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Basic test

static sk_sp<GrTextureProxy> new_deferred_tex(GrResourceProvider* provider, SkBackingFit fit) {
    GrSurfaceDesc desc;
    desc.fFlags = kNone_GrSurfaceFlags;
    desc.fOrigin = kBottomLeft_GrSurfaceOrigin;
    desc.fWidth = 64;
    desc.fHeight = 64;
    desc.fConfig = kRGBA_8888_GrPixelConfig;
    desc.fSampleCnt = 0;

    // Only budgeted proxies get to carry uniqueKeys
    return GrSurfaceProxy::MakeDeferred(provider, desc, fit, SkBudgeted::kYes);
}

static sk_sp<GrTextureProxy> new_deferred_texRT(GrResourceProvider* provider, SkBackingFit fit) {
    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fOrigin = kBottomLeft_GrSurfaceOrigin;
    desc.fWidth = 64;
    desc.fHeight = 64;
    desc.fConfig = kRGBA_8888_GrPixelConfig;
    desc.fSampleCnt = 0;

    // Only budgeted proxies get to carry uniqueKeys
    return GrSurfaceProxy::MakeDeferred(provider, desc, fit, SkBudgeted::kYes);
}

static sk_sp<GrTextureProxy> new_wrapped(GrResourceProvider* provider, SkBackingFit fit) {
    GrSurfaceDesc desc;
    desc.fFlags = kNone_GrSurfaceFlags;
    desc.fOrigin = kBottomLeft_GrSurfaceOrigin;
    desc.fWidth = 64;
    desc.fHeight = 64;
    desc.fConfig = kRGBA_8888_GrPixelConfig;
    desc.fSampleCnt = 0;

    sk_sp<GrTexture> tex;
    if (SkBackingFit::kApprox == fit) {
        tex = sk_sp<GrTexture>(provider->createApproxTexture(desc, 0));
    } else {
        // Only budgeted proxies get to carry uniqueKeys
        tex = provider->createTexture(desc, SkBudgeted::kYes);
    }

    return GrSurfaceProxy::MakeWrapped(std::move(tex), kBottomLeft_GrSurfaceOrigin);
}

// Add wrapped backend case!

// This tests the basic capabilities of the uniquely keyed texture proxies. Does assigning
// and looking them up work, etc.
static void basic_test(GrContext* context, skiatest::Reporter* reporter) {
    GrResourceProvider* provider = context->resourceProvider();
    GrResourceCache* cache = context->getResourceCache();

    REPORTER_ASSERT(reporter, !cache->numUniqueKeyProxies_TestOnly());
    REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());

    int id = 1;
    for (auto fit : { SkBackingFit::kExact, SkBackingFit::kApprox }) {
        for (auto create : { new_deferred_tex, new_deferred_texRT, new_wrapped }) {
            REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());

            sk_sp<GrTextureProxy> proxy(create(provider, fit));
            int startCacheCount = cache->getResourceCount();

            REPORTER_ASSERT(reporter, !proxy->getUniqueKey().isValid());

            GrUniqueKey key;
            GrMakeKeyFromImageID(&key, id, SkIRect::MakeWH(64, 64));
            ++id;

            // Assigning the uniqueKey adds the proxy to the hash but doesn't force instantiation
            REPORTER_ASSERT(reporter, !cache->numUniqueKeyProxies_TestOnly());
            provider->assignUniqueKeyToProxy(key, proxy.get());
            REPORTER_ASSERT(reporter, 1 == cache->numUniqueKeyProxies_TestOnly());
            REPORTER_ASSERT(reporter, startCacheCount == cache->getResourceCount());

            // setUniqueKey had better stick
            REPORTER_ASSERT(reporter, key == proxy->getUniqueKey());

            // We just added it, surely we can find it
            REPORTER_ASSERT(reporter, provider->findProxyByUniqueKey(key,
                                                                     kBottomLeft_GrSurfaceOrigin));
            REPORTER_ASSERT(reporter, 1 == cache->numUniqueKeyProxies_TestOnly());

            // Once instantiated, the backing resource should have the same key
            SkAssertResult(proxy->instantiate(provider));
            const GrUniqueKey& texKey = proxy->priv().peekSurface()->getUniqueKey();
            REPORTER_ASSERT(reporter, texKey.isValid());
            REPORTER_ASSERT(reporter, key == texKey);
            REPORTER_ASSERT(reporter, 1 == cache->getResourceCount());

            // deleting the proxy should delete it from the hash but not the cache
            proxy = nullptr;
            REPORTER_ASSERT(reporter, 0 == cache->numUniqueKeyProxies_TestOnly());
            REPORTER_ASSERT(reporter, 1 == cache->getResourceCount());

            // Refinding the proxy should bring it back to life (from the cache)
            proxy = provider->findProxyByUniqueKey(key, kBottomLeft_GrSurfaceOrigin);
            REPORTER_ASSERT(reporter, proxy);
            REPORTER_ASSERT(reporter, 1 == cache->numUniqueKeyProxies_TestOnly());
            REPORTER_ASSERT(reporter, 1 == cache->getResourceCount());

            // Mega-purging it should remove it from both the hash and the cache
            proxy = nullptr;
            cache->purgeAllUnlocked();
            REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());

            // We can't bring the texture back from perma-death but we can revive the proxy
            proxy = provider->findProxyByUniqueKey(key, kBottomLeft_GrSurfaceOrigin);
            REPORTER_ASSERT(reporter, !proxy);
            REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Invalidation test

// Test if invalidating unique ids on both proxies and the underlying GrTextures operates
// as expected for texture proxies.
static void invalidation_test(GrContext* context, skiatest::Reporter* reporter) {

    GrResourceCache* cache = context->getResourceCache();
    REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());

    sk_sp<SkImage> rasterImg;

    {
        SkImageInfo ii = SkImageInfo::MakeN32Premul(64, 64);

        SkBitmap bm;
        bm.allocPixels(ii);

        rasterImg = SkImage::MakeFromBitmap(bm);
        REPORTER_ASSERT(reporter, 0 == cache->numUniqueKeyProxies_TestOnly());
        REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());
    }

    sk_sp<SkImage> textureImg = rasterImg->makeTextureImage(context, nullptr);
    REPORTER_ASSERT(reporter, 1 == cache->numUniqueKeyProxies_TestOnly());
    REPORTER_ASSERT(reporter, 1 == cache->getResourceCount());

    rasterImg = nullptr;        // this invalidates the uniqueKey

    // this forces the cache to respond to the inval msg
    int maxNum;
    size_t maxBytes;
    context->getResourceCacheLimits(&maxNum, &maxBytes);
    context->setResourceCacheLimits(maxNum-1, maxBytes);

    REPORTER_ASSERT(reporter, 0 == cache->numUniqueKeyProxies_TestOnly());
    REPORTER_ASSERT(reporter, 1 == cache->getResourceCount());

    textureImg = nullptr;
    context->purgeAllUnlockedResources();

    REPORTER_ASSERT(reporter, 0 == cache->numUniqueKeyProxies_TestOnly());
    REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());

}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(TextureProxyTest, reporter, ctxInfo) {
    basic_test(ctxInfo.grContext(), reporter);
    invalidation_test(ctxInfo.grContext(), reporter);
}

#endif
