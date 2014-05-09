/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#if SK_SUPPORT_GPU

#include "GrContextFactory.h"
#include "GrResourceCache.h"
#include "SkGpuDevice.h"
#include "Test.h"

static const int gWidth = 640;
static const int gHeight = 480;

////////////////////////////////////////////////////////////////////////////////
static void test_cache(skiatest::Reporter* reporter,
                       GrContext* context,
                       SkCanvas* canvas) {
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

class TestResource : public GrCacheable {
    static const size_t kDefaultSize = 100;

public:
    SK_DECLARE_INST_COUNT(TestResource);
    TestResource(size_t size = kDefaultSize)
        : fCache(NULL)
        , fToDelete(NULL)
        , fSize(size) {
        ++fAlive;
    }

    ~TestResource() {
        --fAlive;
        if (NULL != fToDelete) {
            // Breaks our little 2-element cycle below.
            fToDelete->setDeleteWhenDestroyed(NULL, NULL);
            fCache->deleteResource(fToDelete->getCacheEntry());
        }
    }

    void setSize(size_t size) {
        fSize = size;
        this->didChangeGpuMemorySize();
    }

    size_t gpuMemorySize() const SK_OVERRIDE { return fSize; }

    bool isValidOnGpu() const SK_OVERRIDE { return true; }

    static int alive() { return fAlive; }

    void setDeleteWhenDestroyed(GrResourceCache* cache, TestResource* resource) {
        fCache = cache;
        fToDelete = resource;
    }

private:
    GrResourceCache* fCache;
    TestResource* fToDelete;
    size_t fSize;
    static int fAlive;

    typedef GrCacheable INHERITED;
};
int TestResource::fAlive = 0;

static void test_purge_invalidated(skiatest::Reporter* reporter, GrContext* context) {
    GrCacheID::Domain domain = GrCacheID::GenerateDomain();
    GrCacheID::Key keyData;
    keyData.fData64[0] = 5;
    keyData.fData64[1] = 18;
    GrResourceKey::ResourceType t = GrResourceKey::GenerateResourceType();
    GrResourceKey key(GrCacheID(domain, keyData), t, 0);

    GrResourceCache cache(5, 30000);

    // Add two resources with the same key that delete each other from the cache when destroyed.
    TestResource* a = new TestResource();
    TestResource* b = new TestResource();
    cache.addResource(key, a);
    cache.addResource(key, b);
    // Circle back.
    a->setDeleteWhenDestroyed(&cache, b);
    b->setDeleteWhenDestroyed(&cache, a);
    a->unref();
    b->unref();

    // Add a third independent resource also with the same key.
    GrCacheable* r = new TestResource();
    cache.addResource(key, r);
    r->unref();

    // Invalidate all three, all three should be purged and destroyed.
    REPORTER_ASSERT(reporter, 3 == TestResource::alive());
    const GrResourceInvalidatedMessage msg = { key };
    SkMessageBus<GrResourceInvalidatedMessage>::Post(msg);
    cache.purgeAsNeeded();
    REPORTER_ASSERT(reporter, 0 == TestResource::alive());
}

static void test_cache_delete_on_destruction(skiatest::Reporter* reporter,
                                             GrContext* context) {
    GrCacheID::Domain domain = GrCacheID::GenerateDomain();
    GrCacheID::Key keyData;
    keyData.fData64[0] = 5;
    keyData.fData64[1] = 0;
    GrResourceKey::ResourceType t = GrResourceKey::GenerateResourceType();

    GrResourceKey key(GrCacheID(domain, keyData), t, 0);

    {
        {
            GrResourceCache cache(3, 30000);
            TestResource* a = new TestResource();
            TestResource* b = new TestResource();
            cache.addResource(key, a);
            cache.addResource(key, b);

            a->setDeleteWhenDestroyed(&cache, b);
            b->setDeleteWhenDestroyed(&cache, a);

            a->unref();
            b->unref();
            REPORTER_ASSERT(reporter, 2 == TestResource::alive());
        }
        REPORTER_ASSERT(reporter, 0 == TestResource::alive());
    }
    {
        GrResourceCache cache(3, 30000);
        TestResource* a = new TestResource();
        TestResource* b = new TestResource();
        cache.addResource(key, a);
        cache.addResource(key, b);

        a->setDeleteWhenDestroyed(&cache, b);
        b->setDeleteWhenDestroyed(&cache, a);

        a->unref();
        b->unref();

        cache.deleteResource(a->getCacheEntry());

        REPORTER_ASSERT(reporter, 0 == TestResource::alive());
    }
}

static void test_resource_size_changed(skiatest::Reporter* reporter,
                                       GrContext* context) {
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
        GrResourceCache cache(2, 300);

        TestResource* a = new TestResource(0);
        a->setSize(100); // Test didChangeGpuMemorySize() when not in the cache.
        cache.addResource(key1, a);
        a->unref();

        TestResource* b = new TestResource(0);
        b->setSize(100);
        cache.addResource(key2, b);
        b->unref();

        REPORTER_ASSERT(reporter, 200 == cache.getCachedResourceBytes());
        REPORTER_ASSERT(reporter, 2 == cache.getCachedResourceCount());

        static_cast<TestResource*>(cache.find(key2))->setSize(200);
        static_cast<TestResource*>(cache.find(key1))->setSize(50);

        REPORTER_ASSERT(reporter, 250 == cache.getCachedResourceBytes());
        REPORTER_ASSERT(reporter, 2 == cache.getCachedResourceCount());
    }

    // Test increasing a resources size beyond the cache budget.
    {
        GrResourceCache cache(2, 300);

        TestResource* a = new TestResource(100);
        cache.addResource(key1, a);
        a->unref();

        TestResource* b = new TestResource(100);
        cache.addResource(key2, b);
        b->unref();

        REPORTER_ASSERT(reporter, 200 == cache.getCachedResourceBytes());
        REPORTER_ASSERT(reporter, 2 == cache.getCachedResourceCount());

        static_cast<TestResource*>(cache.find(key2))->setSize(201);
        REPORTER_ASSERT(reporter, NULL == cache.find(key1));

        REPORTER_ASSERT(reporter, 201 == cache.getCachedResourceBytes());
        REPORTER_ASSERT(reporter, 1 == cache.getCachedResourceCount());
    }

    // Test changing the size of an exclusively-held resource.
    {
        GrResourceCache cache(2, 300);

        TestResource* a = new TestResource(100);
        cache.addResource(key1, a);
        cache.makeExclusive(a->getCacheEntry());

        TestResource* b = new TestResource(100);
        cache.addResource(key2, b);
        b->unref();

        REPORTER_ASSERT(reporter, 200 == cache.getCachedResourceBytes());
        REPORTER_ASSERT(reporter, 2 == cache.getCachedResourceCount());
        REPORTER_ASSERT(reporter, NULL == cache.find(key1));

        a->setSize(200);

        REPORTER_ASSERT(reporter, 300 == cache.getCachedResourceBytes());
        REPORTER_ASSERT(reporter, 2 == cache.getCachedResourceCount());
        // Internal resource cache validation will test the detached size (debug mode only).

        cache.makeNonExclusive(a->getCacheEntry());
        a->unref();

        REPORTER_ASSERT(reporter, 300 == cache.getCachedResourceBytes());
        REPORTER_ASSERT(reporter, 2 == cache.getCachedResourceCount());
        REPORTER_ASSERT(reporter, NULL != cache.find(key1));
        // Internal resource cache validation will test the detached size (debug mode only).
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

        GrTextureDesc desc;
        desc.fConfig = kSkia8888_GrPixelConfig;
        desc.fFlags = kRenderTarget_GrTextureFlagBit;
        desc.fWidth = gWidth;
        desc.fHeight = gHeight;

        SkAutoTUnref<GrTexture> texture(context->createUncachedTexture(desc, NULL, 0));
        SkAutoTUnref<SkGpuDevice> device(SkNEW_ARGS(SkGpuDevice, (context, texture.get())));
        SkCanvas canvas(device.get());

        test_cache(reporter, context, &canvas);
        test_purge_invalidated(reporter, context);
        test_cache_delete_on_destruction(reporter, context);
        test_resource_size_changed(reporter, context);
    }
}

#endif
