/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test.

#include "tests/Test.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkImage.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrResourceCache.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/SkGr.h"
#include "tools/gpu/ManagedBackendTexture.h"

#ifdef SK_DAWN
#include "src/gpu/dawn/GrDawnGpu.h"
#endif

int GrProxyProvider::numUniqueKeyProxies_TestOnly() const {
    return fUniquelyKeyedProxies.count();
}

static constexpr auto kColorType = GrColorType::kRGBA_8888;
static constexpr auto kSize = SkISize::Make(64, 64);

///////////////////////////////////////////////////////////////////////////////////////////////////
// Basic test

static sk_sp<GrTextureProxy> deferred_tex(skiatest::Reporter* reporter,
                                          GrRecordingContext* rContext,
                                          GrProxyProvider* proxyProvider,
                                          SkBackingFit fit) {
    const GrCaps* caps = rContext->priv().caps();

    GrBackendFormat format = caps->getDefaultBackendFormat(kColorType, GrRenderable::kNo);

    sk_sp<GrTextureProxy> proxy =
            proxyProvider->createProxy(format, kSize, GrRenderable::kNo, 1, GrMipmapped::kNo, fit,
                                       SkBudgeted::kYes, GrProtected::kNo);
    // Only budgeted & wrapped external proxies get to carry uniqueKeys
    REPORTER_ASSERT(reporter, !proxy->getUniqueKey().isValid());
    return proxy;
}

static sk_sp<GrTextureProxy> deferred_texRT(skiatest::Reporter* reporter,
                                            GrRecordingContext* rContext,
                                            GrProxyProvider* proxyProvider,
                                            SkBackingFit fit) {
    const GrCaps* caps = rContext->priv().caps();

    GrBackendFormat format = caps->getDefaultBackendFormat(kColorType, GrRenderable::kYes);

    sk_sp<GrTextureProxy> proxy =
            proxyProvider->createProxy(format, kSize, GrRenderable::kYes, 1, GrMipmapped::kNo, fit,
                                       SkBudgeted::kYes, GrProtected::kNo);
    // Only budgeted & wrapped external proxies get to carry uniqueKeys
    REPORTER_ASSERT(reporter, !proxy->getUniqueKey().isValid());
    return proxy;
}

static sk_sp<GrTextureProxy> wrapped(skiatest::Reporter* reporter, GrRecordingContext*,
                                     GrProxyProvider* proxyProvider, SkBackingFit fit) {
    sk_sp<GrTextureProxy> proxy = proxyProvider->testingOnly_createInstantiatedProxy(
            kSize, kColorType, GrRenderable::kNo, 1, fit, SkBudgeted::kYes, GrProtected::kNo);
    // Only budgeted & wrapped external proxies get to carry uniqueKeys
    REPORTER_ASSERT(reporter, !proxy->getUniqueKey().isValid());
    return proxy;
}

static sk_sp<GrTextureProxy> wrapped_with_key(skiatest::Reporter* reporter, GrRecordingContext*,
                                              GrProxyProvider* proxyProvider, SkBackingFit fit) {
    static GrUniqueKey::Domain d = GrUniqueKey::GenerateDomain();
    static int kUniqueKeyData = 0;

    GrUniqueKey key;

    GrUniqueKey::Builder builder(&key, d, 1, nullptr);
    builder[0] = kUniqueKeyData++;
    builder.finish();

    // Only budgeted & wrapped external proxies get to carry uniqueKeys
    sk_sp<GrTextureProxy> proxy = proxyProvider->testingOnly_createInstantiatedProxy(
            kSize, kColorType, GrRenderable::kNo, 1, fit, SkBudgeted::kYes, GrProtected::kNo);
    SkAssertResult(proxyProvider->assignUniqueKeyToProxy(key, proxy.get()));
    REPORTER_ASSERT(reporter, proxy->getUniqueKey().isValid());
    return proxy;
}

static sk_sp<GrTextureProxy> create_wrapped_backend(GrDirectContext* dContext) {
    auto mbet = sk_gpu_test::ManagedBackendTexture::MakeWithoutData(
            dContext,
            kSize.width(),
            kSize.height(),
            GrColorTypeToSkColorType(kColorType),
            GrMipmapped::kNo,
            GrRenderable::kNo,
            GrProtected::kNo);
    if (!mbet) {
        return nullptr;
    }
    GrProxyProvider* proxyProvider = dContext->priv().proxyProvider();
    return proxyProvider->wrapBackendTexture(mbet->texture(),
                                             kBorrow_GrWrapOwnership,
                                             GrWrapCacheable::kYes,
                                             kRead_GrIOType,
                                             mbet->refCountedCallback());
}

// This tests the basic capabilities of the uniquely keyed texture proxies. Does assigning
// and looking them up work, etc.
static void basic_test(GrDirectContext* dContext,
                       skiatest::Reporter* reporter,
                       sk_sp<GrTextureProxy> proxy,
                       int cacheEntriesPerProxy) {
    static int id = 1;

    GrResourceProvider* resourceProvider = dContext->priv().resourceProvider();
    GrProxyProvider* proxyProvider = dContext->priv().proxyProvider();
    GrResourceCache* cache = dContext->priv().getResourceCache();

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
    REPORTER_ASSERT(reporter, proxyProvider->findOrCreateProxyByUniqueKey(key));
    REPORTER_ASSERT(reporter, 1 == proxyProvider->numUniqueKeyProxies_TestOnly());

    int expectedCacheCount = startCacheCount + (proxy->isInstantiated() ? 0 : cacheEntriesPerProxy);

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
        expectedCacheCount -= cacheEntriesPerProxy;
    }
    REPORTER_ASSERT(reporter, 0 == proxyProvider->numUniqueKeyProxies_TestOnly());
    REPORTER_ASSERT(reporter, expectedCacheCount == cache->getResourceCount());

    // If the proxy was cached refinding it should bring it back to life
    proxy = proxyProvider->findOrCreateProxyByUniqueKey(key);
    REPORTER_ASSERT(reporter, proxy);
    REPORTER_ASSERT(reporter, 1 == proxyProvider->numUniqueKeyProxies_TestOnly());
    REPORTER_ASSERT(reporter, expectedCacheCount == cache->getResourceCount());

    // Mega-purging it should remove it from both the hash and the cache
    proxy = nullptr;
    cache->purgeAllUnlocked();
    if (!expectResourceToOutliveProxy) {
        expectedCacheCount -= cacheEntriesPerProxy;
    }
    REPORTER_ASSERT(reporter, expectedCacheCount == cache->getResourceCount());

    // If the texture was deleted then the proxy should no longer be findable. Otherwise, it should
    // be.
    proxy = proxyProvider->findOrCreateProxyByUniqueKey(key);
    REPORTER_ASSERT(reporter, expectResourceToOutliveProxy ? (bool)proxy : !proxy);
    REPORTER_ASSERT(reporter, expectedCacheCount == cache->getResourceCount());

    if (expectResourceToOutliveProxy) {
        proxy.reset();
        GrUniqueKeyInvalidatedMessage msg(texKey, dContext->priv().contextID());
        SkMessageBus<GrUniqueKeyInvalidatedMessage, uint32_t>::Post(msg);
        cache->purgeAsNeeded();
        expectedCacheCount -= cacheEntriesPerProxy;
        proxy = proxyProvider->findOrCreateProxyByUniqueKey(key);
        REPORTER_ASSERT(reporter, !proxy);
        REPORTER_ASSERT(reporter, expectedCacheCount == cache->getResourceCount());
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Invalidation test

// Test if invalidating unique ids operates as expected for texture proxies.
static void invalidation_test(GrDirectContext* dContext,
                              skiatest::Reporter* reporter,
                              int cacheEntriesPerProxy) {

    GrProxyProvider* proxyProvider = dContext->priv().proxyProvider();
    GrResourceCache* cache = dContext->priv().getResourceCache();
    REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());

    sk_sp<SkImage> rasterImg;

    {
        SkImageInfo ii = SkImageInfo::Make(64, 64, kRGBA_8888_SkColorType, kOpaque_SkAlphaType);

        SkBitmap bm;
        bm.allocPixels(ii);

        rasterImg = bm.asImage();
        REPORTER_ASSERT(reporter, 0 == proxyProvider->numUniqueKeyProxies_TestOnly());
        REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());
    }

    // Some of our backends use buffers to do uploads that will live in our resource cache. So we
    // need to account for those extra resources here.
    int bufferResources = 0;
    if (dContext->backend() == GrBackendApi::kDawn ||
        dContext->backend() == GrBackendApi::kVulkan ||
        dContext->backend() == GrBackendApi::kDirect3D ||
        dContext->backend() == GrBackendApi::kMetal) {
        bufferResources = 1;
    }

    sk_sp<SkImage> textureImg = rasterImg->makeTextureImage(dContext);
    REPORTER_ASSERT(reporter, 0 == proxyProvider->numUniqueKeyProxies_TestOnly());
    REPORTER_ASSERT(reporter, cacheEntriesPerProxy + bufferResources == cache->getResourceCount());

    rasterImg = nullptr;        // this invalidates the uniqueKey

    // this forces the cache to respond to the inval msg
    size_t maxBytes = dContext->getResourceCacheLimit();
    dContext->setResourceCacheLimit(maxBytes-1);

    REPORTER_ASSERT(reporter, 0 == proxyProvider->numUniqueKeyProxies_TestOnly());
    REPORTER_ASSERT(reporter, cacheEntriesPerProxy + bufferResources == cache->getResourceCount());

    textureImg = nullptr;

    // For backends that use buffers to upload lets make sure that work has been submit and done
    // before we try to purge all resources.
    dContext->submit(true);

#ifdef SK_DAWN
    // The forced cpu sync in dawn doesn't actually mean the async map will finish thus we may
    // still have a ref on the GrGpuBuffer and it will not get purged by the call below. We dig
    // deep into the dawn gpu to make sure we wait for the async map to finish.
    if (dContext->backend() == GrBackendApi::kDawn) {
        GrDawnGpu* gpu = static_cast<GrDawnGpu*>(dContext->priv().getGpu());
        gpu->waitOnAllBusyStagingBuffers();
    }
#endif

    dContext->priv().testingOnly_purgeAllUnlockedResources();

    REPORTER_ASSERT(reporter, 0 == proxyProvider->numUniqueKeyProxies_TestOnly());
    REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());
}

// Test if invalidating unique ids prior to instantiating operates as expected
static void invalidation_and_instantiation_test(GrDirectContext* dContext,
                                                skiatest::Reporter* reporter,
                                                int cacheEntriesPerProxy) {
    GrProxyProvider* proxyProvider = dContext->priv().proxyProvider();
    GrResourceProvider* resourceProvider = dContext->priv().resourceProvider();
    GrResourceCache* cache = dContext->priv().getResourceCache();
    REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());

    static GrUniqueKey::Domain d = GrUniqueKey::GenerateDomain();
    GrUniqueKey key;
    GrUniqueKey::Builder builder(&key, d, 1, nullptr);
    builder[0] = 0;
    builder.finish();

    // Create proxy, assign unique key
    sk_sp<GrTextureProxy> proxy = deferred_tex(reporter, dContext, proxyProvider,
                                               SkBackingFit::kExact);
    SkAssertResult(proxyProvider->assignUniqueKeyToProxy(key, proxy.get()));

    // Send an invalidation message, which will be sitting in the cache's inbox
    SkMessageBus<GrUniqueKeyInvalidatedMessage, uint32_t>::Post(
            GrUniqueKeyInvalidatedMessage(key, dContext->priv().contextID()));

    REPORTER_ASSERT(reporter, 1 == proxyProvider->numUniqueKeyProxies_TestOnly());
    REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());

    // Instantiate the proxy. This will trigger the message to be processed, so the resulting
    // texture should *not* have the unique key on it!
    SkAssertResult(proxy->instantiate(resourceProvider));

    REPORTER_ASSERT(reporter, !proxy->getUniqueKey().isValid());
    REPORTER_ASSERT(reporter, !proxy->peekTexture()->getUniqueKey().isValid());
    REPORTER_ASSERT(reporter, 0 == proxyProvider->numUniqueKeyProxies_TestOnly());
    REPORTER_ASSERT(reporter, cacheEntriesPerProxy == cache->getResourceCount());

    proxy = nullptr;
    dContext->priv().testingOnly_purgeAllUnlockedResources();

    REPORTER_ASSERT(reporter, 0 == proxyProvider->numUniqueKeyProxies_TestOnly());
    REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(TextureProxyTest, reporter, ctxInfo) {
    auto direct = ctxInfo.directContext();
    GrProxyProvider* proxyProvider = direct->priv().proxyProvider();
    GrResourceCache* cache = direct->priv().getResourceCache();

    REPORTER_ASSERT(reporter, !proxyProvider->numUniqueKeyProxies_TestOnly());
    REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());

    // As we transition to using attachments instead of GrTextures and GrRenderTargets individual
    // proxy instansiations may add multiple things to the cache. There would be an entry for the
    // GrTexture/GrRenderTarget and entries for one or more attachments.
    int cacheEntriesPerProxy = 1;
    // We currently only have attachments on the vulkan backend
    if (direct->backend() == GrBackend::kVulkan) {
        cacheEntriesPerProxy++;
        // If we ever have a test with multisamples this would have an additional attachment as
        // well.
    }

    for (auto fit : { SkBackingFit::kExact, SkBackingFit::kApprox }) {
        for (auto create : { deferred_tex, deferred_texRT, wrapped, wrapped_with_key }) {
            REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());
            basic_test(direct, reporter, create(reporter, direct, proxyProvider, fit),
                       cacheEntriesPerProxy);
        }

        REPORTER_ASSERT(reporter, 0 == cache->getResourceCount());
        cache->purgeAllUnlocked();
    }

    basic_test(direct, reporter, create_wrapped_backend(direct), cacheEntriesPerProxy);

    invalidation_test(direct, reporter, cacheEntriesPerProxy);
    invalidation_and_instantiation_test(direct, reporter, cacheEntriesPerProxy);
}
