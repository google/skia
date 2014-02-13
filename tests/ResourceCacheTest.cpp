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

    size_t initialCacheSize = context->getGpuTextureCacheBytes();

    int oldMaxNum;
    size_t oldMaxBytes;
    context->getTextureCacheLimits(&oldMaxNum, &oldMaxBytes);

    // Set the cache limits so we can fit 10 "src" images and the
    // max number of textures doesn't matter
    size_t maxCacheSize = initialCacheSize + 10*srcSize;
    context->setTextureCacheLimits(1000, maxCacheSize);

    SkBitmap readback;
    readback.allocN32Pixels(size.width(), size.height());

    for (int i = 0; i < 100; ++i) {
        canvas->drawBitmap(src, 0, 0);
        canvas->readPixels(size, &readback);

        // "modify" the src texture
        src.notifyPixelsChanged();

        size_t curCacheSize = context->getGpuTextureCacheBytes();

        // we should never go over the size limit
        REPORTER_ASSERT(reporter, curCacheSize <= maxCacheSize);
    }

    context->setTextureCacheLimits(oldMaxNum, oldMaxBytes);
}

class TestResource : public GrResource {
public:
    SK_DECLARE_INST_COUNT(TestResource);
    explicit TestResource(GrGpu* gpu)
        : INHERITED(gpu, false)
        , fCache(NULL)
        , fToDelete(NULL) {
        ++fAlive;
    }

    ~TestResource() {
        --fAlive;
        if (NULL != fToDelete) {
            // Breaks our little 2-element cycle below.
            fToDelete->setDeleteWhenDestroyed(NULL, NULL);
            fCache->deleteResource(fToDelete->getCacheEntry());
        }
        this->release();
    }

    size_t sizeInBytes() const SK_OVERRIDE { return 100; }

    static int alive() { return fAlive; }

    void setDeleteWhenDestroyed(GrResourceCache* cache, TestResource* resource) {
        fCache = cache;
        fToDelete = resource;
    }

private:
    GrResourceCache* fCache;
    TestResource* fToDelete;
    static int fAlive;

    typedef GrResource INHERITED;
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
    TestResource* a = new TestResource(context->getGpu());
    TestResource* b = new TestResource(context->getGpu());
    cache.addResource(key, a);
    cache.addResource(key, b);
    // Circle back.
    a->setDeleteWhenDestroyed(&cache, b);
    b->setDeleteWhenDestroyed(&cache, a);
    a->unref();
    b->unref();

    // Add a third independent resource also with the same key.
    GrResource* r = new TestResource(context->getGpu());
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
            TestResource* a = new TestResource(context->getGpu());
            TestResource* b = new TestResource(context->getGpu());
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
        TestResource* a = new TestResource(context->getGpu());
        TestResource* b = new TestResource(context->getGpu());
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
    }
}

#endif
