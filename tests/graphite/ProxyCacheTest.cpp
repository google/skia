/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkBitmap.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/GpuTypesPriv.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/ProxyCache.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "tools/DecodeUtils.h"
#include "tools/Resources.h"
#include "tools/graphite/GraphiteTestContext.h"

#include <thread>

namespace skgpu::graphite {

// This test exercises the basic MessageBus behavior of the ProxyCache by manually inserting an
// SkBitmap into the proxy cache and then changing its contents. This simple test should create
// an IDChangeListener that will remove the entry in the cache when the bitmap is changed and
// the resulting message processed.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(ProxyCacheTest1, r, context, CtsEnforcement::kApiLevel_V) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    ProxyCache* proxyCache = recorder->priv().proxyCache();

    SkBitmap bitmap;
    bool success = ToolUtils::GetResourceAsBitmap("images/mandrill_128.png", &bitmap);
    REPORTER_ASSERT(r, success);
    if (!success) {
        return;
    }

    REPORTER_ASSERT(r, proxyCache->numCached() == 0);

    sk_sp<TextureProxy> proxy = proxyCache->findOrCreateCachedProxy(recorder.get(), bitmap,
                                                                    "ProxyCacheTestTexture");

    REPORTER_ASSERT(r, proxyCache->numCached() == 1);

    bitmap.eraseColor(SK_ColorBLACK);

    proxyCache->forceProcessInvalidKeyMsgs();

    REPORTER_ASSERT(r, proxyCache->numCached() == 0);
}

// This test checks that, if the same bitmap is added to two separate ProxyCaches, when it is
// changed, both of the ProxyCaches will receive the message.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(ProxyCacheTest2, r, context, CtsEnforcement::kApiLevel_V) {
    std::unique_ptr<Recorder> recorder1 = context->makeRecorder();
    ProxyCache* proxyCache1 = recorder1->priv().proxyCache();
    std::unique_ptr<Recorder> recorder2 = context->makeRecorder();
    ProxyCache* proxyCache2 = recorder2->priv().proxyCache();

    SkBitmap bitmap;
    bool success = ToolUtils::GetResourceAsBitmap("images/mandrill_128.png", &bitmap);
    REPORTER_ASSERT(r, success);
    if (!success) {
        return;
    }

    REPORTER_ASSERT(r, proxyCache1->numCached() == 0);
    REPORTER_ASSERT(r, proxyCache2->numCached() == 0);

    sk_sp<TextureProxy> proxy1 = proxyCache1->findOrCreateCachedProxy(recorder1.get(), bitmap,
                                                                      "ProxyCacheTestTexture");
    sk_sp<TextureProxy> proxy2 = proxyCache2->findOrCreateCachedProxy(recorder2.get(), bitmap,
                                                                      "ProxyCacheTestTexture");

    REPORTER_ASSERT(r, proxyCache1->numCached() == 1);
    REPORTER_ASSERT(r, proxyCache2->numCached() == 1);

    bitmap.eraseColor(SK_ColorBLACK);

    proxyCache1->forceProcessInvalidKeyMsgs();
    proxyCache2->forceProcessInvalidKeyMsgs();

    REPORTER_ASSERT(r, proxyCache1->numCached() == 0);
    REPORTER_ASSERT(r, proxyCache2->numCached() == 0);
}

namespace {

struct ProxyCacheSetup {
    bool valid() const {
        return !fBitmap1.empty() && !fBitmap2.empty() && fProxy1 && fProxy2;
    }

    SkBitmap fBitmap1;
    sk_sp<TextureProxy> fProxy1;
    SkBitmap fBitmap2;
    sk_sp<TextureProxy> fProxy2;

    skgpu::StdSteadyClock::time_point fTimeBetweenProxyCreation;
    skgpu::StdSteadyClock::time_point fTimeAfterAllProxyCreation;
};

ProxyCacheSetup setup_test(Context* context,
                           skiatest::graphite::GraphiteTestContext* testContext,
                           Recorder* recorder,
                           skiatest::Reporter* r) {
    ProxyCache* proxyCache = recorder->priv().proxyCache();

    ProxyCacheSetup setup;

    bool success1 = ToolUtils::GetResourceAsBitmap("images/mandrill_32.png", &setup.fBitmap1);
    bool success2 = ToolUtils::GetResourceAsBitmap("images/mandrill_64.png", &setup.fBitmap2);
    if (!success1 || !success2) {
        return {};
    }

    REPORTER_ASSERT(r, proxyCache->numCached() == 0);

    setup.fProxy1 = proxyCache->findOrCreateCachedProxy(recorder, setup.fBitmap1,
                                                        "ProxyCacheTestTexture");
    REPORTER_ASSERT(r, proxyCache->numCached() == 1);

    {
        // Ensure proxy1's Texture is created (and timestamped) at this time
        auto recording = recorder->snap();
        context->insertRecording({ recording.get() });
        context->submit(SyncToCpu::kYes);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    setup.fTimeBetweenProxyCreation = skgpu::StdSteadyClock::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(2));

    setup.fProxy2 = proxyCache->findOrCreateCachedProxy(recorder, setup.fBitmap2,
                                                        "ProxyCacheTestTexture");
    REPORTER_ASSERT(r, proxyCache->numCached() == 2);

    {
        // Ensure proxy2's Texture is created (and timestamped) at this time
        auto recording = recorder->snap();
        context->insertRecording({ recording.get() });
        testContext->syncedSubmit(context);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    setup.fTimeAfterAllProxyCreation = skgpu::StdSteadyClock::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(2));

    return setup;
}

} // anonymous namespace

// This test exercises the ProxyCache's freeUniquelyHeld method.
DEF_CONDITIONAL_GRAPHITE_TEST_FOR_ALL_CONTEXTS(ProxyCacheTest4,
                                               r,
                                               context,
                                               testContext,
                                               true,
                                               CtsEnforcement::kApiLevel_V) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    ProxyCache* proxyCache = recorder->priv().proxyCache();

    ProxyCacheSetup setup = setup_test(context, testContext, recorder.get(), r);
    REPORTER_ASSERT(r, setup.valid());
    if (!setup.valid()) {
        return;
    }

    proxyCache->forceFreeUniquelyHeld();
    REPORTER_ASSERT(r, proxyCache->numCached() == 2);

    setup.fProxy1.reset();
    proxyCache->forceFreeUniquelyHeld();
    REPORTER_ASSERT(r, proxyCache->numCached() == 1);

    setup.fProxy2.reset();
    proxyCache->forceFreeUniquelyHeld();
    REPORTER_ASSERT(r, proxyCache->numCached() == 0);
}

// This test exercises the ProxyCache's purgeProxiesNotUsedSince method.
DEF_CONDITIONAL_GRAPHITE_TEST_FOR_ALL_CONTEXTS(ProxyCacheTest5,
                                               r,
                                               context,
                                               testContext,
                                               true,
                                               CtsEnforcement::kApiLevel_V) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    ProxyCache* proxyCache = recorder->priv().proxyCache();

    ProxyCacheSetup setup = setup_test(context, testContext, recorder.get(), r);
    REPORTER_ASSERT(r, setup.valid());
    if (!setup.valid()) {
        return;
    }

    proxyCache->forcePurgeProxiesNotUsedSince(setup.fTimeBetweenProxyCreation);
    REPORTER_ASSERT(r, proxyCache->numCached() == 1);

    sk_sp<TextureProxy> test = proxyCache->find(setup.fBitmap1);
    REPORTER_ASSERT(r, !test);   // proxy1 should've been purged

    proxyCache->forcePurgeProxiesNotUsedSince(setup.fTimeAfterAllProxyCreation);
    REPORTER_ASSERT(r, proxyCache->numCached() == 0);
}

// This test simply verifies that the ProxyCache is correctly updating the Resource's
// last access time stamp.
DEF_CONDITIONAL_GRAPHITE_TEST_FOR_ALL_CONTEXTS(ProxyCacheTest6,
                                               r,
                                               context,
                                               testContext,
                                               true,
                                               CtsEnforcement::kApiLevel_V) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    ProxyCache* proxyCache = recorder->priv().proxyCache();

    ProxyCacheSetup setup = setup_test(context, testContext, recorder.get(), r);
    REPORTER_ASSERT(r, setup.valid());
    if (!setup.valid()) {
        return;
    }

    // update proxy1's timestamp
    sk_sp<TextureProxy> test = proxyCache->findOrCreateCachedProxy(recorder.get(), setup.fBitmap1,
                                                                   "ProxyCacheTestTexture");
    REPORTER_ASSERT(r, test == setup.fProxy1);

    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    auto timeAfterProxy1Update = skgpu::StdSteadyClock::now();

    proxyCache->forcePurgeProxiesNotUsedSince(setup.fTimeBetweenProxyCreation);
    REPORTER_ASSERT(r, proxyCache->numCached() == 2);

    proxyCache->forcePurgeProxiesNotUsedSince(setup.fTimeAfterAllProxyCreation);
    REPORTER_ASSERT(r, proxyCache->numCached() == 1);

    test = proxyCache->find(setup.fBitmap2);
    REPORTER_ASSERT(r, !test);   // proxy2 should've been purged

    proxyCache->forcePurgeProxiesNotUsedSince(timeAfterProxy1Update);
    REPORTER_ASSERT(r, proxyCache->numCached() == 0);
}

// Verify that the ProxyCache's purgeProxiesNotUsedSince method can clear out multiple proxies.
DEF_CONDITIONAL_GRAPHITE_TEST_FOR_ALL_CONTEXTS(ProxyCacheTest7,
                                               r,
                                               context,
                                               testContext,
                                               true,
                                               CtsEnforcement::kApiLevel_V) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    ProxyCache* proxyCache = recorder->priv().proxyCache();

    ProxyCacheSetup setup = setup_test(context, testContext, recorder.get(), r);
    REPORTER_ASSERT(r, setup.valid());
    if (!setup.valid()) {
        return;
    }

    proxyCache->forcePurgeProxiesNotUsedSince(setup.fTimeAfterAllProxyCreation);
    REPORTER_ASSERT(r, proxyCache->numCached() == 0);
}

// Verify that the ProxyCache's freeUniquelyHeld behavior is working in the ResourceCache.
DEF_CONDITIONAL_GRAPHITE_TEST_FOR_ALL_CONTEXTS(ProxyCacheTest8,
                                               r,
                                               context,
                                               testContext,
                                               true,
                                               CtsEnforcement::kApiLevel_V) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    ResourceCache* resourceCache = recorder->priv().resourceCache();
    ProxyCache* proxyCache = recorder->priv().proxyCache();

    resourceCache->setMaxBudget(0);

    ProxyCacheSetup setup = setup_test(context, testContext, recorder.get(), r);
    REPORTER_ASSERT(r, setup.valid());
    if (!setup.valid()) {
        return;
    }

    resourceCache->forcePurgeAsNeeded();

    REPORTER_ASSERT(r, proxyCache->numCached() == 2);

    setup.fProxy1.reset();
    proxyCache->forceProcessInvalidKeyMsgs();

    // unreffing fProxy1 and forcing message processing shouldn't purge proxy1 from the cache
    sk_sp<TextureProxy> test = proxyCache->find(setup.fBitmap1);
    REPORTER_ASSERT(r, test);
    test.reset();

    resourceCache->forcePurgeAsNeeded();

    REPORTER_ASSERT(r, proxyCache->numCached() == 1);
    test = proxyCache->find(setup.fBitmap1);
    REPORTER_ASSERT(r, !test);   // proxy1 should've been purged

    setup.fProxy2.reset();
    proxyCache->forceProcessInvalidKeyMsgs();

    // unreffing fProxy2 and forcing message processing shouldn't purge proxy2 from the cache
    test = proxyCache->find(setup.fBitmap2);
    REPORTER_ASSERT(r, test);
    test.reset();

    resourceCache->forcePurgeAsNeeded();

    REPORTER_ASSERT(r, proxyCache->numCached() == 0);
}

// Verify that the ProxyCache's purgeProxiesNotUsedSince behavior is working when triggered from
// ResourceCache.
DEF_CONDITIONAL_GRAPHITE_TEST_FOR_ALL_CONTEXTS(ProxyCacheTest9,
                                               r,
                                               context,
                                               testContext,
                                               true,
                                               CtsEnforcement::kApiLevel_V) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    ResourceCache* resourceCache = recorder->priv().resourceCache();
    ProxyCache* proxyCache = recorder->priv().proxyCache();

    ProxyCacheSetup setup = setup_test(context, testContext, recorder.get(), r);
    REPORTER_ASSERT(r, setup.valid());
    if (!setup.valid()) {
        return;
    }

    REPORTER_ASSERT(r, setup.fProxy1->isInstantiated());
    REPORTER_ASSERT(r, setup.fProxy2->isInstantiated());

    if (!setup.fProxy1->texture() || !setup.fProxy2->texture()) {
        return;
    }

    // Clear out resources used to setup bitmap proxies so we can track things easier.
    resourceCache->setMaxBudget(0);
    resourceCache->setMaxBudget(256 * (1 << 20));

    REPORTER_ASSERT(r, proxyCache->numCached() == 2);
    int baselineResourceCount = resourceCache->getResourceCount();
    // When buffer maps are async it can take extra time for buffers to be returned to the cache.
    if (context->priv().caps()->bufferMapsAreAsync()) {
        // We expect at least 2 textures (and possibly buffers).
        REPORTER_ASSERT(r, baselineResourceCount >= 2);
    } else {
        REPORTER_ASSERT(r, baselineResourceCount == 2);
    }
    // Force a command buffer ref on the second proxy in the cache so it can't be purged immediately
    setup.fProxy2->texture()->refCommandBuffer();

    Resource* proxy1ResourcePtr = setup.fProxy1->texture();
    Resource* proxy2ResourcePtr = setup.fProxy2->texture();

    setup.fProxy1.reset();
    setup.fProxy2.reset();
    REPORTER_ASSERT(r, proxyCache->numCached() == 2);

    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    auto timeAfterProxyCreation = skgpu::StdSteadyClock::now();

    // This should trigger both proxies to be purged from the ProxyCache. Neither proxy should be
    // released from the ResourceCache since their timestamp gets reset when returned to the
    // ResourceCache. However, the first proxy should be purgeable and the second not since it still
    // has a command buffer ref.
    resourceCache->purgeResourcesNotUsedSince(timeAfterProxyCreation);

    REPORTER_ASSERT(r, proxyCache->numCached() == 0);
    REPORTER_ASSERT(r, resourceCache->getResourceCount() == baselineResourceCount);
    REPORTER_ASSERT(r, resourceCache->topOfPurgeableQueue() == proxy1ResourcePtr);

    // Removing the command buffer ref and returning proxy2Resource to the cache should cause it to
    // become purgeable.
    proxy2ResourcePtr->unrefCommandBuffer();
    resourceCache->forceProcessReturnedResources();

    REPORTER_ASSERT(r, proxyCache->numCached() == 0);
    REPORTER_ASSERT(r, resourceCache->getResourceCount() == baselineResourceCount);
    REPORTER_ASSERT(r, resourceCache->topOfPurgeableQueue() == proxy1ResourcePtr);
    REPORTER_ASSERT(r, resourceCache->testingInPurgeableQueue(proxy2ResourcePtr));
}

static sk_sp<TextureProxy> find_or_create_by_key(Recorder* recorder, int id, bool* regenerated) {
    *regenerated = false;

    skgpu::UniqueKey key;
    {
        static const skgpu::UniqueKey::Domain kTestDomain = UniqueKey::GenerateDomain();
        UniqueKey::Builder builder(&key, kTestDomain, 1, "TestExplicitKey");
        builder[0] = id;
    }

    struct Context {
        int id;
        bool* regenerated;
    } params { id, regenerated };

    return recorder->priv().proxyCache()->findOrCreateCachedProxy(
            recorder, key, &params,
            [](const void* context) {
                const Context* params = static_cast<const Context*>(context);
                *params->regenerated = true;

                SkBitmap bm;
                if (params->id == 1) {
                    if (!ToolUtils::GetResourceAsBitmap("images/mandrill_32.png", &bm)) {
                        return SkBitmap();
                    }
                } else if (!ToolUtils::GetResourceAsBitmap("images/mandrill_64.png", &bm)) {
                    return SkBitmap();
                }
                return bm;
            });
}

// Verify that the ProxyCache's explicit keying only generates the bitmaps as needed.
DEF_CONDITIONAL_GRAPHITE_TEST_FOR_ALL_CONTEXTS(ProxyCacheTest10,
                                               r,
                                               context,
                                               testContext,
                                               true,
                                               CtsEnforcement::kApiLevel_V) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    ProxyCache* proxyCache = recorder->priv().proxyCache();

    REPORTER_ASSERT(r, proxyCache->numCached() == 0);

    bool regenerated;
    sk_sp<TextureProxy> proxy1 = find_or_create_by_key(recorder.get(), 1, &regenerated);
    REPORTER_ASSERT(r, proxy1 && proxy1->dimensions().width() == 32);
    REPORTER_ASSERT(r, regenerated);

    sk_sp<TextureProxy> proxy2 = find_or_create_by_key(recorder.get(), 2, &regenerated);
    REPORTER_ASSERT(r, proxy2 && proxy2->dimensions().width() == 64);
    REPORTER_ASSERT(r, regenerated);

    REPORTER_ASSERT(r, proxyCache->numCached() == 2);

    // These cached proxies shouldn't be deleted because we hold local refs still
    proxyCache->forceFreeUniquelyHeld();
    REPORTER_ASSERT(r, proxyCache->numCached() == 2);

    // Cache hit should not invoke the bitmap generation function.
    sk_sp<TextureProxy> proxy1b = find_or_create_by_key(recorder.get(), 1, &regenerated);
    REPORTER_ASSERT(r, proxy1.get() == proxy1b.get());
    REPORTER_ASSERT(r, !regenerated);

    proxy1.reset();
    proxy1b.reset();
    proxy2.reset();
    (void) recorder->snap(); // Dump pending commands to release internal refs to the cached proxies

    // Now the cache should clean the cache entries up
    proxyCache->forceFreeUniquelyHeld();
    REPORTER_ASSERT(r, proxyCache->numCached() == 0);

    // And regeneration functions as expected
    proxy1 = find_or_create_by_key(recorder.get(), 1, &regenerated);
    REPORTER_ASSERT(r, proxy1 && proxy1->dimensions().width() == 32);
    REPORTER_ASSERT(r, regenerated);
}

}  // namespace skgpu::graphite
