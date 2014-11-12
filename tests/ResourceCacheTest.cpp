/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#if SK_SUPPORT_GPU

#include "GrContext.h"
#include "GrContextFactory.h"
#include "GrGpu.h"
#include "GrResourceCache.h"
#include "GrResourceCache2.h"
#include "SkCanvas.h"
#include "SkSurface.h"
#include "Test.h"

static const int gWidth = 640;
static const int gHeight = 480;

////////////////////////////////////////////////////////////////////////////////
static void test_cache(skiatest::Reporter* reporter, GrContext* context, SkCanvas* canvas) {
    const SkIRect size = SkIRect::MakeWH(gWidth, gHeight);

    SkBitmap src;
    src.allocN32Pixels(size.width(), size.height());
    src.eraseColor(SK_ColorBLACK);
    size_t srcSize = src.getSize();

    size_t initialCacheSize;
    context->getResourceCacheUsage(NULL, &initialCacheSize);

    int oldMaxNum;
    size_t oldMaxBytes;
    context->getResourceCacheLimits(&oldMaxNum, &oldMaxBytes);

    // Set the cache limits so we can fit 10 "src" images and the
    // max number of textures doesn't matter
    size_t maxCacheSize = initialCacheSize + 10*srcSize;
    context->setResourceCacheLimits(1000, maxCacheSize);

    SkBitmap readback;
    readback.allocN32Pixels(size.width(), size.height());

    for (int i = 0; i < 100; ++i) {
        canvas->drawBitmap(src, 0, 0);
        canvas->readPixels(size, &readback);

        // "modify" the src texture
        src.notifyPixelsChanged();

        size_t curCacheSize;
        context->getResourceCacheUsage(NULL, &curCacheSize);

        // we should never go over the size limit
        REPORTER_ASSERT(reporter, curCacheSize <= maxCacheSize);
    }

    context->setResourceCacheLimits(oldMaxNum, oldMaxBytes);
}

class TestResource : public GrGpuResource {
    static const size_t kDefaultSize = 100;

public:
    SK_DECLARE_INST_COUNT(TestResource);
    TestResource(GrGpu* gpu)
        : INHERITED(gpu, false)
        , fCache(NULL)
        , fToDelete(NULL)
        , fSize(kDefaultSize) {
        ++fNumAlive;
        this->registerWithCache();
    }

    TestResource(GrGpu* gpu, const GrResourceKey& scratchKey)
        : INHERITED(gpu, false)
        , fCache(NULL)
        , fToDelete(NULL)
        , fSize(kDefaultSize) {
        this->setScratchKey(scratchKey);
        ++fNumAlive;
        this->registerWithCache();
    }

    ~TestResource() {
        --fNumAlive;
        if (fToDelete) {
            // Breaks our little 2-element cycle below.
            fToDelete->setDeleteWhenDestroyed(NULL, NULL);
            fCache->deleteResource(fToDelete->cacheAccess().getCacheEntry());
        }
        this->release();
    }

    void setSize(size_t size) {
        fSize = size;
        this->didChangeGpuMemorySize();
    }

    size_t gpuMemorySize() const SK_OVERRIDE { return fSize; }

    static int NumAlive() { return fNumAlive; }

    void setDeleteWhenDestroyed(GrResourceCache* cache, TestResource* resource) {
        fCache = cache;
        fToDelete = resource;
    }

private:
    GrResourceCache* fCache;
    TestResource* fToDelete;
    size_t fSize;
    static int fNumAlive;

    typedef GrGpuResource INHERITED;
};
int TestResource::fNumAlive = 0;

static void test_duplicate_scratch_key(skiatest::Reporter* reporter) {
    SkAutoTUnref<GrContext> context(GrContext::CreateMockContext());
    REPORTER_ASSERT(reporter, SkToBool(context));
    if (NULL == context) {
        return;
    }
    context->setResourceCacheLimits(5, 30000);
    GrResourceCache* cache = context->getResourceCache();
    SkDEBUGCODE(GrResourceCache2* cache2 = context->getResourceCache2();)
    cache->purgeAllUnlocked();
    SkASSERT(0 == cache->getCachedResourceCount() && 0 == cache->getCachedResourceBytes());

    GrCacheID::Key keyData;
    memset(&keyData, 0, sizeof(keyData));
    GrCacheID::Domain domain = GrResourceKey::ScratchDomain();
    GrResourceKey::ResourceType t = GrResourceKey::GenerateResourceType();
    GrResourceKey scratchKey(GrCacheID(domain, keyData), t, 0);

    // Create two resources that have the same scratch key.
    TestResource* a = new TestResource(context->getGpu(), scratchKey);
    TestResource* b = new TestResource(context->getGpu(), scratchKey);
    a->setSize(11);
    b->setSize(12);
    // Scratch resources are registered with GrResourceCache2 just by existing. There are 2.
    SkDEBUGCODE(REPORTER_ASSERT(reporter, 2 == cache2->countScratchEntriesForKey(scratchKey));)

    REPORTER_ASSERT(reporter, cache->addResource(scratchKey, a));

    SkDEBUGCODE(REPORTER_ASSERT(reporter, 2 == cache2->countScratchEntriesForKey(scratchKey));)

    // Can't add the same resource twice.
    REPORTER_ASSERT(reporter, !cache->addResource(scratchKey, a));
    REPORTER_ASSERT(reporter, 1 == cache->getCachedResourceCount());
    REPORTER_ASSERT(reporter, a->gpuMemorySize() == cache->getCachedResourceBytes());
    SkDEBUGCODE(REPORTER_ASSERT(reporter, 2 == cache2->countScratchEntriesForKey(scratchKey));)

    // Add a second with the same key.
    REPORTER_ASSERT(reporter, cache->addResource(scratchKey, b));
    REPORTER_ASSERT(reporter, 2 == cache->getCachedResourceCount());
    REPORTER_ASSERT(reporter, a->gpuMemorySize() + b->gpuMemorySize() ==
                              cache->getCachedResourceBytes());
    REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());
    SkDEBUGCODE(REPORTER_ASSERT(reporter, 2 == cache2->countScratchEntriesForKey(scratchKey));)

    // Our refs mean that the resources are non purgable.
    cache->purgeAllUnlocked();
    REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());
    REPORTER_ASSERT(reporter, 2 == cache->getCachedResourceCount());

    // Unref but don't purge
    a->unref();
    b->unref();
    REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());
    SkDEBUGCODE(REPORTER_ASSERT(reporter, 2 == cache2->countScratchEntriesForKey(scratchKey));)

    // Purge again. This time resources should be purgable.
    cache->purgeAllUnlocked();
    REPORTER_ASSERT(reporter, 0 == TestResource::NumAlive());
    REPORTER_ASSERT(reporter, 0 == cache->getCachedResourceCount());
    SkDEBUGCODE(REPORTER_ASSERT(reporter, 0 == cache2->countScratchEntriesForKey(scratchKey));)
}

static void test_duplicate_content_key(skiatest::Reporter* reporter) {
    SkAutoTUnref<GrContext> context(GrContext::CreateMockContext());
    REPORTER_ASSERT(reporter, SkToBool(context));
    if (NULL == context) {
        return;
    }
    context->setResourceCacheLimits(5, 30000);
    GrResourceCache* cache = context->getResourceCache();
    cache->purgeAllUnlocked();
    SkASSERT(0 == cache->getCachedResourceCount() && 0 == cache->getCachedResourceBytes());

    GrCacheID::Domain domain = GrCacheID::GenerateDomain();
    GrCacheID::Key keyData;
    memset(&keyData, 0, sizeof(keyData));
    GrResourceKey::ResourceType t = GrResourceKey::GenerateResourceType();
    GrResourceKey key(GrCacheID(domain, keyData), t, 0);
    

    // Create two resources that we will attempt to register with the same content key.
    TestResource* a = new TestResource(context->getGpu());
    TestResource* b = new TestResource(context->getGpu());
    a->setSize(11);
    b->setSize(12);
    REPORTER_ASSERT(reporter, cache->addResource(key, a));
    // Can't add the same or another resource with the same key.
    REPORTER_ASSERT(reporter, !cache->addResource(key, a));
    REPORTER_ASSERT(reporter, !cache->addResource(key, b));
    REPORTER_ASSERT(reporter, 1 == cache->getCachedResourceCount());
    REPORTER_ASSERT(reporter, a->gpuMemorySize() == cache->getCachedResourceBytes());
    REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());

    b->unref();
    cache->purgeAllUnlocked();
    a->setSize(10);
    REPORTER_ASSERT(reporter, 1 == cache->getCachedResourceCount());
    REPORTER_ASSERT(reporter, 1 == TestResource::NumAlive());

    a->unref();
    cache->purgeAllUnlocked();
    REPORTER_ASSERT(reporter, 0 == cache->getCachedResourceCount());
    REPORTER_ASSERT(reporter, 0 == cache->getCachedResourceBytes());
    REPORTER_ASSERT(reporter, 0 == TestResource::NumAlive());
}

static void test_purge_invalidated(skiatest::Reporter* reporter) {
    SkAutoTUnref<GrContext> context(GrContext::CreateMockContext());
    REPORTER_ASSERT(reporter, SkToBool(context));
    if (NULL == context) {
        return;
    }

    GrCacheID::Domain domain = GrCacheID::GenerateDomain();
    GrCacheID::Key keyData;
    memset(&keyData, 0, sizeof(keyData));

    GrResourceKey::ResourceType t = GrResourceKey::GenerateResourceType();

    keyData.fData64[0] = 1;
    GrResourceKey key1(GrCacheID(domain, keyData), t, 0);
    keyData.fData64[0] = 2;
    GrResourceKey key2(GrCacheID(domain, keyData), t, 0);
    keyData.fData64[0] = 3;
    GrResourceKey key3(GrCacheID(domain, keyData), t, 0);
    
    context->setResourceCacheLimits(5, 30000);
    GrResourceCache* cache = context->getResourceCache();
    GrResourceCache2* cache2 = context->getResourceCache2();
    cache->purgeAllUnlocked();
    SkASSERT(0 == cache->getCachedResourceCount() && 0 == cache->getCachedResourceBytes());

    // Add three resources to the cache.
    TestResource* a = new TestResource(context->getGpu());
    TestResource* b = new TestResource(context->getGpu());
    TestResource* c = new TestResource(context->getGpu());
    cache->addResource(key1, a);
    cache->addResource(key2, b);
    cache->addResource(key3, c);
    a->unref();
    b->unref();
    c->unref();

    REPORTER_ASSERT(reporter, cache2->hasContentKey(key1));
    REPORTER_ASSERT(reporter, cache2->hasContentKey(key2));
    REPORTER_ASSERT(reporter, cache2->hasContentKey(key3));

    // Invalidate two of the three, they should be purged and destroyed.
    REPORTER_ASSERT(reporter, 3 == TestResource::NumAlive());
    const GrResourceInvalidatedMessage msg1 = { key1 };
    SkMessageBus<GrResourceInvalidatedMessage>::Post(msg1);
    const GrResourceInvalidatedMessage msg2 = { key2 };
    SkMessageBus<GrResourceInvalidatedMessage>::Post(msg2);
    cache->purgeAsNeeded();
#if 0 // Disabled until reimplemented in GrResourceCache2.
    REPORTER_ASSERT(reporter, 1 == TestResource::NumAlive());
    REPORTER_ASSERT(reporter, !cache2->hasContentKey(key1));
    REPORTER_ASSERT(reporter, !cache2->hasContentKey(key2));
    REPORTER_ASSERT(reporter, cache2->hasContentKey(key3));
#endif

    // Invalidate the third.
    const GrResourceInvalidatedMessage msg3 = { key3 };
    SkMessageBus<GrResourceInvalidatedMessage>::Post(msg3);
    cache->purgeAsNeeded();
#if 0 // Disabled until reimplemented in GrResourceCache2.
    REPORTER_ASSERT(reporter, 0 == TestResource::NumAlive());
    REPORTER_ASSERT(reporter, !cache2->hasContentKey(key3));
#endif
}

static void test_cache_delete_on_destruction(skiatest::Reporter* reporter) {
    SkAutoTUnref<GrContext> context(GrContext::CreateMockContext());
    REPORTER_ASSERT(reporter, SkToBool(context));
    if (NULL == context) {
        return;
    }

    GrCacheID::Domain domain = GrCacheID::GenerateDomain();
    GrCacheID::Key keyData;
    memset(&keyData, 0, sizeof(keyData));
    GrResourceKey::ResourceType t = GrResourceKey::GenerateResourceType();

    keyData.fData64[0] = 1;
    GrResourceKey key1(GrCacheID(domain, keyData), t, 0);

    keyData.fData64[0] = 2;
    GrResourceKey key2(GrCacheID(domain, keyData), t, 0);

    {
        context->setResourceCacheLimits(3, 30000);
        GrResourceCache* cache = context->getResourceCache();
        cache->purgeAllUnlocked();
        SkASSERT(0 == cache->getCachedResourceCount() && 0 == cache->getCachedResourceBytes());

        TestResource* a = new TestResource(context->getGpu());
        TestResource* b = new TestResource(context->getGpu());
        cache->addResource(key1, a);
        cache->addResource(key2, b);

        a->setDeleteWhenDestroyed(cache, b);
        b->setDeleteWhenDestroyed(cache, a);

        a->unref();
        b->unref();

        REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());

        cache->purgeAllUnlocked();
        REPORTER_ASSERT(reporter, 0 == TestResource::NumAlive());
    }
    {
        context->setResourceCacheLimits(3, 30000);
        GrResourceCache* cache = context->getResourceCache();
        cache->purgeAllUnlocked();
        SkASSERT(0 == cache->getCachedResourceCount() && 0 == cache->getCachedResourceBytes());

        TestResource* a = new TestResource(context->getGpu());
        TestResource* b = new TestResource(context->getGpu());
        cache->addResource(key1, a);
        cache->addResource(key2, b);

        a->setDeleteWhenDestroyed(cache, b);
        b->setDeleteWhenDestroyed(cache, a);

        a->unref();
        b->unref();

        cache->deleteResource(a->cacheAccess().getCacheEntry());
        REPORTER_ASSERT(reporter, 0 == TestResource::NumAlive());
    }
}

static void test_resource_size_changed(skiatest::Reporter* reporter) {
    SkAutoTUnref<GrContext> context(GrContext::CreateMockContext());
    REPORTER_ASSERT(reporter, SkToBool(context));
    if (NULL == context) {
        return;
    }

    GrCacheID::Domain domain = GrCacheID::GenerateDomain();
    GrResourceKey::ResourceType t = GrResourceKey::GenerateResourceType();

    GrCacheID::Key key1Data;
    key1Data.fData64[0] = 0;
    key1Data.fData64[1] = 0;
    GrResourceKey key1(GrCacheID(domain, key1Data), t, 0);

    GrCacheID::Key key2Data;
    key2Data.fData64[0] = 1;
    key2Data.fData64[1] = 0;
    GrResourceKey key2(GrCacheID(domain, key2Data), t, 0);

    // Test changing resources sizes (both increase & decrease).
    {
        context->setResourceCacheLimits(3, 30000);
        GrResourceCache* cache = context->getResourceCache();
        GrResourceCache2* cache2 = context->getResourceCache2();
        cache->purgeAllUnlocked();
        SkASSERT(0 == cache->getCachedResourceCount() && 0 == cache->getCachedResourceBytes());

        TestResource* a = new TestResource(context->getGpu());
        a->setSize(100); // Test didChangeGpuMemorySize() when not in the cache.
        cache->addResource(key1, a);
        a->unref();

        TestResource* b = new TestResource(context->getGpu());
        b->setSize(100);
        cache->addResource(key2, b);
        b->unref();

        REPORTER_ASSERT(reporter, 200 == cache->getCachedResourceBytes());
        REPORTER_ASSERT(reporter, 2 == cache->getCachedResourceCount());
        {
            SkAutoTUnref<TestResource> find2(static_cast<TestResource*>(cache2->findAndRefContentResource(key2)));
            find2->setSize(200);
            SkAutoTUnref<TestResource> find1(static_cast<TestResource*>(cache2->findAndRefContentResource(key1)));
            find1->setSize(50);
        }

        REPORTER_ASSERT(reporter, 250 == cache->getCachedResourceBytes());
        REPORTER_ASSERT(reporter, 2 == cache->getCachedResourceCount());
    }

    // Test increasing a resources size beyond the cache budget.
    {
        context->setResourceCacheLimits(2, 300);
        GrResourceCache* cache = context->getResourceCache();
        GrResourceCache2* cache2 = context->getResourceCache2();
        cache->purgeAllUnlocked();
        SkASSERT(0 == cache->getCachedResourceCount() && 0 == cache->getCachedResourceBytes());

        TestResource* a = new TestResource(context->getGpu());
        a->setSize(100);
        cache->addResource(key1, a);
        a->unref();

        TestResource* b = new TestResource(context->getGpu());
        b->setSize(100);
        cache->addResource(key2, b);
        b->unref();

        REPORTER_ASSERT(reporter, 200 == cache->getCachedResourceBytes());
        REPORTER_ASSERT(reporter, 2 == cache->getCachedResourceCount());

        {
            SkAutoTUnref<TestResource> find2(static_cast<TestResource*>(cache2->findAndRefContentResource(key2)));
            find2->setSize(201);
        }
        REPORTER_ASSERT(reporter, !cache2->hasContentKey(key1));

        REPORTER_ASSERT(reporter, 201 == cache->getCachedResourceBytes());
        REPORTER_ASSERT(reporter, 1 == cache->getCachedResourceCount());
    }
}

////////////////////////////////////////////////////////////////////////////////
DEF_GPUTEST(ResourceCache, reporter, factory) {
    for (int type = 0; type < GrContextFactory::kLastGLContextType; ++type) {
        GrContextFactory::GLContextType glType = static_cast<GrContextFactory::GLContextType>(type);
        if (!GrContextFactory::IsRenderingGLContext(glType)) {
            continue;
        }
        GrContext* context = factory->get(glType);
        if (NULL == context) {
            continue;
        }
        GrSurfaceDesc desc;
        desc.fConfig = kSkia8888_GrPixelConfig;
        desc.fFlags = kRenderTarget_GrSurfaceFlag;
        desc.fWidth = gWidth;
        desc.fHeight = gHeight;
        SkImageInfo info = SkImageInfo::MakeN32Premul(gWidth, gHeight);
        SkAutoTUnref<SkSurface> surface(SkSurface::NewRenderTarget(context, info));
        test_cache(reporter, context, surface->getCanvas());
    }

    // The below tests create their own mock contexts.
    test_duplicate_content_key(reporter);
    test_duplicate_scratch_key(reporter);
    test_purge_invalidated(reporter);
    test_cache_delete_on_destruction(reporter);
    test_resource_size_changed(reporter);
}

#endif
