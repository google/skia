/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test.

#include "tests/Test.h"

#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrTexture.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrResourceCache.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrTextureProxy.h"

#include "include/core/SkImage.h"
#include "src/gpu/SkGr.h"

int GrProxyProvider::numUniqueKeyProxies_TestOnly() const {
    return fUniquelyKeyedProxies.count();
}

static constexpr auto kColorType = GrColorType::kRGBA_8888;
static constexpr auto kSize = SkISize::Make(64, 64);
static GrSurfaceDesc make_desc() {
    GrSurfaceDesc desc;
    desc.fWidth = kSize.width();
    desc.fHeight = kSize.height();
    desc.fConfig = GrColorTypeToPixelConfig(kColorType);
    return desc;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Basic test

static sk_sp<GrTextureProxy> deferred_tex(skiatest::Reporter* reporter, GrContext* ctx,
                                          GrProxyProvider* proxyProvider, SkBackingFit fit) {
    const GrCaps* caps = ctx->priv().caps();

    const GrSurfaceDesc desc = make_desc();
    GrBackendFormat format = caps->getDefaultBackendFormat(kColorType, GrRenderable::kNo);

    sk_sp<GrTextureProxy> proxy = proxyProvider->createProxy(format, desc, GrRenderable::kNo, 1,
                                                             kBottomLeft_GrSurfaceOrigin, fit,
                                                             SkBudgeted::kYes, GrProtected::kNo);
    // Only budgeted & wrapped external proxies get to carry uniqueKeys
    REPORTER_ASSERT(reporter, !proxy->getUniqueKey().isValid());
    return proxy;
}

static sk_sp<GrTextureProxy> deferred_texRT(skiatest::Reporter* reporter, GrContext* ctx,
                                            GrProxyProvider* proxyProvider, SkBackingFit fit) {
    const GrCaps* caps = ctx->priv().caps();

    const GrSurfaceDesc desc = make_desc();

    GrBackendFormat format = caps->getDefaultBackendFormat(kColorType, GrRenderable::kYes);

    sk_sp<GrTextureProxy> proxy = proxyProvider->createProxy(format, desc, GrRenderable::kYes, 1,
                                                             kBottomLeft_GrSurfaceOrigin, fit,
                                                             SkBudgeted::kYes, GrProtected::kNo);
    // Only budgeted & wrapped external proxies get to carry uniqueKeys
    REPORTER_ASSERT(reporter, !proxy->getUniqueKey().isValid());
    return proxy;
}

static sk_sp<GrTextureProxy> wrapped(skiatest::Reporter* reporter, GrContext* ctx,
                                     GrProxyProvider* proxyProvider, SkBackingFit fit) {
    sk_sp<GrTextureProxy> proxy = proxyProvider->testingOnly_createInstantiatedProxy(
            kSize, kColorType, GrRenderable::kNo, 1, kBottomLeft_GrSurfaceOrigin, fit,
            SkBudgeted::kYes, GrProtected::kNo);
    // Only budgeted & wrapped external proxies get to carry uniqueKeys
    REPORTER_ASSERT(reporter, !proxy->getUniqueKey().isValid());
    return proxy;
}

static sk_sp<GrTextureProxy> wrapped_with_key(skiatest::Reporter* reporter, GrContext* ctx,
                                              GrProxyProvider* proxyProvider, SkBackingFit fit) {
    static GrUniqueKey::Domain d = GrUniqueKey::GenerateDomain();
    static int kUniqueKeyData = 0;

    GrUniqueKey key;

    GrUniqueKey::Builder builder(&key, d, 1, nullptr);
    builder[0] = kUniqueKeyData++;
    builder.finish();

    // Only budgeted & wrapped external proxies get to carry uniqueKeys
    sk_sp<GrTextureProxy> proxy = proxyProvider->testingOnly_createInstantiatedProxy(
            kSize, kColorType, GrRenderable::kNo, 1, kBottomLeft_GrSurfaceOrigin, fit,
            SkBudgeted::kYes, GrProtected::kNo);
    SkAssertResult(proxyProvider->assignUniqueKeyToProxy(key, proxy.get()));
    REPORTER_ASSERT(reporter, proxy->getUniqueKey().isValid());
    return proxy;
}

static sk_sp<GrTextureProxy> create_wrapped_backend(GrContext* context, SkBackingFit fit,
                                                    sk_sp<GrTexture>* backingSurface) {
    GrProxyProvider* proxyProvider = context->priv().proxyProvider();
    GrResourceProvider* resourceProvider = context->priv().resourceProvider();

    const GrSurfaceDesc desc = make_desc();
    GrBackendFormat format =
            proxyProvider->caps()->getDefaultBackendFormat(kColorType, GrRenderable::kYes);

    *backingSurface = resourceProvider->createTexture(desc, format, GrRenderable::kNo, 1,
                                                      SkBudgeted::kNo, GrProtected::kNo,
                                                      GrResourceProvider::Flags::kNoPendingIO);
    if (!(*backingSurface)) {
        return nullptr;
    }

    GrBackendTexture backendTex = (*backingSurface)->getBackendTexture();

    return proxyProvider->wrapBackendTexture(backendTex, GrColorType::kRGBA_8888,
                                             kBottomLeft_GrSurfaceOrigin, kBorrow_GrWrapOwnership,
                                             GrWrapCacheable::kYes, kRead_GrIOType);
}


// This tests the basic capabilities of the uniquely keyed texture proxies. Does assigning
// and looking them up work, etc.
static void basic_test(GrContext* context,
                       skiatest::Reporter* reporter,
                       sk_sp<GrTextureProxy> proxy) {
    static int id = 1;

    GrResourceProvider* resourceProvider = context->priv().resourceProvider();
    GrProxyProvider* proxyProvider = context->priv().proxyProvider();
    GrResourceCache* cache = context->priv().getResourceCache();

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

    int expectedCacheCount = startCacheCount + (proxy->isInstantiated() ? 0 : 1);

    // Once instantiated, the backing resource should have the same key
    SkAssertResult(proxy->instantiate(resourceProvider));
    const GrUniqueKey texKey = proxy->peekSurface()->getUniqueKey();
    REPORTER_ASSERT(reporter, texKey.isValid());
    REPORTER_ASSERT(reporter, key == texKey);

    // An Unbudgeted-cacheable resource will not get purged when a proxy with the same key is
    // deleted.
    bool expectResourceToOutliveProxy = proxy->peekSurface()->resourcePriv().budgetedType() ==
                                        GrBudgetedType::kUnbudgetedCacheable;

    // An Unbudgeted-uncacheable resource is never kept alive if it's ref cnt reaches zero even if
    // it has a key.
    bool expectDeletingProxyToDeleteResource =
            proxy->peekSurface()->resourcePriv().budgetedType() ==
            GrBudgetedType::kUnbudgetedUncacheable;

    // deleting the proxy should delete it from the hash but not the cache
    proxy = nullptr;
    if (expectDeletingProxyToDeleteResource) {
        expectedCacheCount -= 1;
    }
    REPORTER_ASSERT(reporter, 0 == proxyProvider->numUniqueKeyProxies_TestOnly());
    REPORTER_ASSERT(reporter, expectedCacheCount == cache->getResourceCount());

    // If the proxy was cached refinding it should bring it back to life
    proxy = proxyProvider->findOrCreateProxyByUniqueKey(key, kBottomLeft_GrSurfaceOrigin);
    REPORTER_ASSERT(reporter, proxy);
    REPORTER_ASSERT(reporter, 1 == proxyProvider->numUniqueKeyProxies_TestOnly());
    REPORTER_ASSERT(reporter, expectedCacheCount == cache->getResourceCount());

    // Mega-purging it should remove it from both the hash and the cache
    proxy = nullptr;
    cache->purgeAllUnlocked();
    if (!expectResourceToOutliveProxy) {
        expectedCacheCount--;
    }
    REPORTER_ASSERT(reporter, expectedCacheCount == cache->getResourceCount());

    // If the texture was deleted then the proxy should no longer be findable. Otherwise, it should
    // be.
    proxy = proxyProvider->findOrCreateProxyByUniqueKey(key, kBottomLeft_GrSurfaceOrigin);
    REPORTER_ASSERT(reporter, expectResourceToOutliveProxy ? (bool)proxy : !proxy);
    REPORTER_ASSERT(reporter, expectedCacheCount == cache->getResourceCount());

    if (expectResourceToOutliveProxy) {
        proxy.reset();
        GrUniqueKeyInvalidatedMessage msg(texKey, context->priv().contextID());
        SkMessageBus<GrUniqueKeyInvalidatedMessage>::Post(msg);
        cache->purgeAsNeeded();
        expectedCacheCount--;
        proxy = proxyProvider->findOrCreateProxyByUniqueKey(key, kBottomLeft_GrSurfaceOrigin);
        REPORTER_ASSERT(reporter, !proxy);
        REPORTER_ASSERT(reporter, expectedCacheCount == cache->getResourceCount());
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Invalidation test

// Test if invalidating unique ids operates as expected for texture proxies.
static void invalidation_test(GrContext* context, skiatest::Reporter* reporter) {

    GrProxyProvider* proxyProvider = context->priv().proxyProvider();
    GrResourceCache* cache = context->priv().getResourceCache();
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
    context->priv().testingOnly_purgeAllUnlockedResources();

    REPORTER_ASSERT(reporter, 0 == proxyProvider->numUniqueKeyProxies_TestOnly());
    REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());
}

// Test if invalidating unique ids prior to instantiating operates as expected
static void invalidation_and_instantiation_test(GrContext* context, skiatest::Reporter* reporter) {
    GrProxyProvider* proxyProvider = context->priv().proxyProvider();
    GrResourceProvider* resourceProvider = context->priv().resourceProvider();
    GrResourceCache* cache = context->priv().getResourceCache();
    REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());

    static GrUniqueKey::Domain d = GrUniqueKey::GenerateDomain();
    GrUniqueKey key;
    GrUniqueKey::Builder builder(&key, d, 1, nullptr);
    builder[0] = 0;
    builder.finish();

    // Create proxy, assign unique key
    sk_sp<GrTextureProxy> proxy = deferred_tex(reporter, context, proxyProvider,
                                               SkBackingFit::kExact);
    SkAssertResult(proxyProvider->assignUniqueKeyToProxy(key, proxy.get()));

    // Send an invalidation message, which will be sitting in the cache's inbox
    SkMessageBus<GrUniqueKeyInvalidatedMessage>::Post(
            GrUniqueKeyInvalidatedMessage(key, context->priv().contextID()));

    REPORTER_ASSERT(reporter, 1 == proxyProvider->numUniqueKeyProxies_TestOnly());
    REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());

    // Instantiate the proxy. This will trigger the message to be processed, so the resulting
    // texture should *not* have the unique key on it!
    SkAssertResult(proxy->instantiate(resourceProvider));

    REPORTER_ASSERT(reporter, !proxy->getUniqueKey().isValid());
    REPORTER_ASSERT(reporter, !proxy->peekTexture()->getUniqueKey().isValid());
    REPORTER_ASSERT(reporter, 0 == proxyProvider->numUniqueKeyProxies_TestOnly());
    REPORTER_ASSERT(reporter, 1 == cache->getResourceCount());

    proxy = nullptr;
    context->priv().testingOnly_purgeAllUnlockedResources();

    REPORTER_ASSERT(reporter, 0 == proxyProvider->numUniqueKeyProxies_TestOnly());
    REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(TextureProxyTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    GrProxyProvider* proxyProvider = context->priv().proxyProvider();
    GrResourceCache* cache = context->priv().getResourceCache();

    REPORTER_ASSERT(reporter, !proxyProvider->numUniqueKeyProxies_TestOnly());
    REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());

    for (auto fit : { SkBackingFit::kExact, SkBackingFit::kApprox }) {
        for (auto create : { deferred_tex, deferred_texRT, wrapped, wrapped_with_key }) {
            REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());
            basic_test(context, reporter, create(reporter, context, proxyProvider, fit));
        }

        REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());
        sk_sp<GrTexture> backingTex;
        sk_sp<GrTextureProxy> proxy = create_wrapped_backend(context, fit, &backingTex);
        basic_test(context, reporter, std::move(proxy));

        backingTex = nullptr;
        cache->purgeAllUnlocked();
    }

    invalidation_test(context, reporter);
    invalidation_and_instantiation_test(context, reporter);
}
