/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test.

#include "Test.h"

#include "GrBackendSurface.h"
#include "GrContextPriv.h"
#include "GrResourceCache.h"
#include "GrProxyProvider.h"
#include "GrResourceProvider.h"
#include "GrTest.h"
#include "GrTexture.h"
#include "GrTextureProxy.h"

#include "SkGr.h"
#include "SkImage.h"

int GrProxyProvider::numUniqueKeyProxies_TestOnly() const {
    return fUniquelyKeyedProxies.count();
}

static GrSurfaceDesc make_desc(GrSurfaceDescFlags descFlags) {
    GrSurfaceDesc desc;
    desc.fFlags = descFlags;
    desc.fWidth = 64;
    desc.fHeight = 64;
    desc.fConfig = kRGBA_8888_GrPixelConfig;
    desc.fSampleCnt = 1;

    return desc;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Basic test

static sk_sp<GrTextureProxy> deferred_tex(skiatest::Reporter* reporter,
                                          GrProxyProvider* proxyProvider, SkBackingFit fit) {
    const GrSurfaceDesc desc = make_desc(kNone_GrSurfaceFlags);

    sk_sp<GrTextureProxy> proxy =
            proxyProvider->createProxy(desc, kBottomLeft_GrSurfaceOrigin, fit, SkBudgeted::kYes);
    // Only budgeted & wrapped external proxies get to carry uniqueKeys
    REPORTER_ASSERT(reporter, !proxy->getUniqueKey().isValid());
    return proxy;
}

static sk_sp<GrTextureProxy> deferred_texRT(skiatest::Reporter* reporter,
                                            GrProxyProvider* proxyProvider, SkBackingFit fit) {
    const GrSurfaceDesc desc = make_desc(kRenderTarget_GrSurfaceFlag);

    sk_sp<GrTextureProxy> proxy =
            proxyProvider->createProxy(desc, kBottomLeft_GrSurfaceOrigin, fit, SkBudgeted::kYes);
    // Only budgeted & wrapped external proxies get to carry uniqueKeys
    REPORTER_ASSERT(reporter, !proxy->getUniqueKey().isValid());
    return proxy;
}

static sk_sp<GrTextureProxy> wrapped(skiatest::Reporter* reporter,
                                     GrProxyProvider* proxyProvider, SkBackingFit fit) {
    const GrSurfaceDesc desc = make_desc(kNone_GrSurfaceFlags);

    sk_sp<GrTextureProxy> proxy = proxyProvider->createInstantiatedProxy(
            desc, kBottomLeft_GrSurfaceOrigin, fit, SkBudgeted::kYes);
    // Only budgeted & wrapped external proxies get to carry uniqueKeys
    REPORTER_ASSERT(reporter, !proxy->getUniqueKey().isValid());
    return proxy;
}

static sk_sp<GrTextureProxy> wrapped_with_key(skiatest::Reporter* reporter,
                                              GrProxyProvider* proxyProvider, SkBackingFit fit) {
    static GrUniqueKey::Domain d = GrUniqueKey::GenerateDomain();
    static int kUniqueKeyData = 0;

    GrUniqueKey key;

    GrUniqueKey::Builder builder(&key, d, 1, nullptr);
    builder[0] = kUniqueKeyData++;
    builder.finish();

    const GrSurfaceDesc desc = make_desc(kNone_GrSurfaceFlags);

    // Only budgeted & wrapped external proxies get to carry uniqueKeys
    sk_sp<GrTextureProxy> proxy = proxyProvider->createInstantiatedProxy(
            desc, kBottomLeft_GrSurfaceOrigin, fit, SkBudgeted::kYes);
    SkAssertResult(proxyProvider->assignUniqueKeyToProxy(key, proxy.get()));
    REPORTER_ASSERT(reporter, proxy->getUniqueKey().isValid());
    return proxy;
}

static sk_sp<GrTextureProxy> create_wrapped_backend(GrContext* context, SkBackingFit fit,
                                                    sk_sp<GrTexture>* backingSurface) {
    GrProxyProvider* proxyProvider = context->contextPriv().proxyProvider();
    GrResourceProvider* resourceProvider = context->contextPriv().resourceProvider();

    const GrSurfaceDesc desc = make_desc(kNone_GrSurfaceFlags);

    *backingSurface = resourceProvider->createTexture(desc, SkBudgeted::kNo);
    if (!(*backingSurface)) {
        return nullptr;
    }

    GrBackendTexture backendTex = (*backingSurface)->getBackendTexture();
    backendTex.setPixelConfig(desc.fConfig);

    return proxyProvider->wrapBackendTexture(backendTex, kBottomLeft_GrSurfaceOrigin);
}


// This tests the basic capabilities of the uniquely keyed texture proxies. Does assigning
// and looking them up work, etc.
static void basic_test(GrContext* context,
                       skiatest::Reporter* reporter,
                       sk_sp<GrTextureProxy> proxy, bool proxyIsCached) {
    static int id = 1;

    GrResourceProvider* resourceProvider = context->contextPriv().resourceProvider();
    GrProxyProvider* proxyProvider = context->contextPriv().proxyProvider();
    GrResourceCache* cache = context->contextPriv().getResourceCache();

    int startCacheCount = cache->getResourceCount();

    GrUniqueKey key;
    if (proxy->getUniqueKey().isValid()) {
        key = proxy->getUniqueKey();
    } else {
        GrMakeKeyFromImageID(&key, id, SkIRect::MakeWH(64, 64));
        ++id;

        // Assigning the uniqueKey adds the proxy to the hash but doesn't force instantiation
        REPORTER_ASSERT(reporter, !proxyProvider->numUniqueKeyProxies_TestOnly());
        SkAssertResult(proxyProvider->assignUniqueKeyToProxy(key, proxy.get()));
    }

    REPORTER_ASSERT(reporter, 1 == proxyProvider->numUniqueKeyProxies_TestOnly());
    REPORTER_ASSERT(reporter, startCacheCount == cache->getResourceCount());

    // setUniqueKey had better stick
    REPORTER_ASSERT(reporter, key == proxy->getUniqueKey());

    // We just added it, surely we can find it
    REPORTER_ASSERT(reporter, proxyProvider->findOrCreateProxyByUniqueKey(
                                                                key, kBottomLeft_GrSurfaceOrigin));
    REPORTER_ASSERT(reporter, 1 == proxyProvider->numUniqueKeyProxies_TestOnly());

    // Once instantiated, the backing resource should have the same key
    SkAssertResult(proxy->instantiate(resourceProvider));
    const GrUniqueKey& texKey = proxy->priv().peekSurface()->getUniqueKey();
    REPORTER_ASSERT(reporter, texKey.isValid());
    REPORTER_ASSERT(reporter, key == texKey);
    if (proxyIsCached) {
        REPORTER_ASSERT(reporter, 1 == cache->getResourceCount());
    }

    // deleting the proxy should delete it from the hash but not the cache
    proxy = nullptr;
    REPORTER_ASSERT(reporter, 0 == proxyProvider->numUniqueKeyProxies_TestOnly());
    REPORTER_ASSERT(reporter, 1 == cache->getResourceCount());

    // If the proxy was cached refinding it should bring it back to life
    proxy = proxyProvider->findOrCreateProxyByUniqueKey(key, kBottomLeft_GrSurfaceOrigin);
    if (proxyIsCached) {
        REPORTER_ASSERT(reporter, proxy);
        REPORTER_ASSERT(reporter, 1 == proxyProvider->numUniqueKeyProxies_TestOnly());
    } else {
        REPORTER_ASSERT(reporter, !proxy);
        REPORTER_ASSERT(reporter, 0 == proxyProvider->numUniqueKeyProxies_TestOnly());
    }
    REPORTER_ASSERT(reporter, 1 == cache->getResourceCount());

    // Mega-purging it should remove it from both the hash and the cache
    proxy = nullptr;
    cache->purgeAllUnlocked();
    if (proxyIsCached) {
        REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());
    } else {
        REPORTER_ASSERT(reporter, 1 == cache->getResourceCount());
    }

    // We can bring neither the texture nor proxy back from perma-death
    proxy = proxyProvider->findOrCreateProxyByUniqueKey(key, kBottomLeft_GrSurfaceOrigin);
    REPORTER_ASSERT(reporter, !proxy);
    if (proxyIsCached) {
        REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());
    } else {
        REPORTER_ASSERT(reporter, 1 == cache->getResourceCount());
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Invalidation test

// Test if invalidating unique ids operates as expected for texture proxies.
static void invalidation_test(GrContext* context, skiatest::Reporter* reporter) {

    GrProxyProvider* proxyProvider = context->contextPriv().proxyProvider();
    GrResourceCache* cache = context->contextPriv().getResourceCache();
    REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());

    sk_sp<SkImage> rasterImg;

    {
        SkImageInfo ii = SkImageInfo::Make(64, 64, kRGBA_8888_SkColorType, kOpaque_SkAlphaType);

        SkBitmap bm;
        bm.allocPixels(ii);

        rasterImg = SkImage::MakeFromBitmap(bm);
        REPORTER_ASSERT(reporter, 0 == proxyProvider->numUniqueKeyProxies_TestOnly());
        REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());
    }

    sk_sp<SkImage> textureImg = rasterImg->makeTextureImage(context, nullptr);
    REPORTER_ASSERT(reporter, 1 == proxyProvider->numUniqueKeyProxies_TestOnly());
    REPORTER_ASSERT(reporter, 1 == cache->getResourceCount());

    rasterImg = nullptr;        // this invalidates the uniqueKey

    // this forces the cache to respond to the inval msg
    int maxNum;
    size_t maxBytes;
    context->getResourceCacheLimits(&maxNum, &maxBytes);
    context->setResourceCacheLimits(maxNum-1, maxBytes);

    REPORTER_ASSERT(reporter, 0 == proxyProvider->numUniqueKeyProxies_TestOnly());
    REPORTER_ASSERT(reporter, 1 == cache->getResourceCount());

    textureImg = nullptr;
    context->contextPriv().purgeAllUnlockedResources_ForTesting();

    REPORTER_ASSERT(reporter, 0 == proxyProvider->numUniqueKeyProxies_TestOnly());
    REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());
}

// Test if invalidating unique ids prior to instantiating operates as expected
static void invalidation_and_instantiation_test(GrContext* context, skiatest::Reporter* reporter) {
    GrProxyProvider* proxyProvider = context->contextPriv().proxyProvider();
    GrResourceProvider* resourceProvider = context->contextPriv().resourceProvider();
    GrResourceCache* cache = context->contextPriv().getResourceCache();
    REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());

    static GrUniqueKey::Domain d = GrUniqueKey::GenerateDomain();
    GrUniqueKey key;
    GrUniqueKey::Builder builder(&key, d, 1, nullptr);
    builder[0] = 0;
    builder.finish();

    // Create proxy, assign unique key
    sk_sp<GrTextureProxy> proxy = deferred_tex(reporter, proxyProvider, SkBackingFit::kExact);
    SkAssertResult(proxyProvider->assignUniqueKeyToProxy(key, proxy.get()));

    // Send an invalidation message, which will be sitting in the cache's inbox
    SkMessageBus<GrUniqueKeyInvalidatedMessage>::Post(
            GrUniqueKeyInvalidatedMessage(key, context->uniqueID()));

    REPORTER_ASSERT(reporter, 1 == proxyProvider->numUniqueKeyProxies_TestOnly());
    REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());

    // Instantiate the proxy. This will trigger the message to be processed, so the resulting
    // texture should *not* have the unique key on it!
    SkAssertResult(proxy->instantiate(resourceProvider));

    REPORTER_ASSERT(reporter, !proxy->getUniqueKey().isValid());
    REPORTER_ASSERT(reporter, !proxy->priv().peekTexture()->getUniqueKey().isValid());
    REPORTER_ASSERT(reporter, 0 == proxyProvider->numUniqueKeyProxies_TestOnly());
    REPORTER_ASSERT(reporter, 1 == cache->getResourceCount());

    proxy = nullptr;
    context->contextPriv().purgeAllUnlockedResources_ForTesting();

    REPORTER_ASSERT(reporter, 0 == proxyProvider->numUniqueKeyProxies_TestOnly());
    REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(TextureProxyTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    GrProxyProvider* proxyProvider = context->contextPriv().proxyProvider();
    GrResourceCache* cache = context->contextPriv().getResourceCache();

    REPORTER_ASSERT(reporter, !proxyProvider->numUniqueKeyProxies_TestOnly());
    REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());

    for (auto fit : { SkBackingFit::kExact, SkBackingFit::kApprox }) {
        for (auto create : { deferred_tex, deferred_texRT, wrapped, wrapped_with_key }) {
            REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());
            basic_test(context, reporter, create(reporter, proxyProvider, fit), true);
        }

        REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());
        sk_sp<GrTexture> backingTex;
        sk_sp<GrTextureProxy> proxy = create_wrapped_backend(context, fit, &backingTex);
        basic_test(context, reporter, std::move(proxy), false);

        backingTex = nullptr;
        cache->purgeAllUnlocked();
    }

    invalidation_test(context, reporter);
    invalidation_and_instantiation_test(context, reporter);
}
