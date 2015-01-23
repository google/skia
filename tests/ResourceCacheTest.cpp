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
#include "GrResourceCache2.h"
#include "SkCanvas.h"
#include "SkGr.h"
#include "SkMessageBus.h"
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
    enum ScratchConstructor { kScratchConstructor };
public:
    SK_DECLARE_INST_COUNT(TestResource);
    /** Property that distinctly categorizes the resource.
     * For example, textures have width, height, ... */
    enum SimulatedProperty { kProperty1_SimulatedProperty, kProperty2_SimulatedProperty };

    TestResource(GrGpu* gpu, size_t size, GrGpuResource::LifeCycle lifeCycle)
        : INHERITED(gpu, lifeCycle)
        , fToDelete(NULL)
        , fSize(size)
        , fProperty(kProperty1_SimulatedProperty) {
        ++fNumAlive;
        this->registerWithCache();
    }

    TestResource(GrGpu* gpu, GrGpuResource::LifeCycle lifeCycle)
        : INHERITED(gpu, lifeCycle)
        , fToDelete(NULL)
        , fSize(kDefaultSize)
        , fProperty(kProperty1_SimulatedProperty) {
        ++fNumAlive;
        this->registerWithCache();
    }

    TestResource(GrGpu* gpu)
        : INHERITED(gpu, kCached_LifeCycle)
        , fToDelete(NULL)
        , fSize(kDefaultSize)
        , fProperty(kProperty1_SimulatedProperty) {
        ++fNumAlive;
        this->registerWithCache();
    }

    static TestResource* CreateScratchTestResource(GrGpu* gpu,
                                                   SimulatedProperty property,
                                                   bool cached = true) {
        return SkNEW_ARGS(TestResource, (gpu, property, cached, kScratchConstructor));
    }

    ~TestResource() {
        --fNumAlive;
        SkSafeUnref(fToDelete);
    }

    void setSize(size_t size) {
        fSize = size;
        this->didChangeGpuMemorySize();
    }

    static int NumAlive() { return fNumAlive; }

    void setUnrefWhenDestroyed(TestResource* resource) {
        SkRefCnt_SafeAssign(fToDelete, resource);
    }

    static void ComputeScratchKey(SimulatedProperty property, GrScratchKey* key) {
        static GrScratchKey::ResourceType t = GrScratchKey::GenerateResourceType();
        GrScratchKey::Builder builder(key, t, kScratchKeyFieldCnt);
        for (int i = 0; i < kScratchKeyFieldCnt; ++i) {
            builder[i] = static_cast<uint32_t>(i + property);
        }
    }

    static size_t ExpectedScratchKeySize() {
        return sizeof(uint32_t) * (kScratchKeyFieldCnt + GrScratchKey::kMetaDataCnt);
    }

private:
    static const int kScratchKeyFieldCnt = 6;

    TestResource(GrGpu* gpu, SimulatedProperty property, bool cached, ScratchConstructor)
        : INHERITED(gpu, cached ? kCached_LifeCycle : kUncached_LifeCycle)
        , fToDelete(NULL)
        , fSize(kDefaultSize)
        , fProperty(property) {
        GrScratchKey scratchKey;
        ComputeScratchKey(fProperty, &scratchKey);
        this->setScratchKey(scratchKey);
        ++fNumAlive;
        this->registerWithCache();
    }

    size_t onGpuMemorySize() const SK_OVERRIDE { return fSize; }

    TestResource* fToDelete;
    size_t fSize;
    static int fNumAlive;
    SimulatedProperty fProperty;
    typedef GrGpuResource INHERITED;
};
int TestResource::fNumAlive = 0;

class Mock {
public:
    Mock(int maxCnt, size_t maxBytes) {
        fContext.reset(GrContext::CreateMockContext());
        SkASSERT(fContext);
        fContext->setResourceCacheLimits(maxCnt, maxBytes);
        GrResourceCache2* cache2 = fContext->getResourceCache2();
        cache2->purgeAllUnlocked();
        SkASSERT(0 == cache2->getResourceCount() && 0 == cache2->getResourceBytes());
    }

    GrResourceCache2* cache() { return fContext->getResourceCache2(); }

    GrContext* context() { return fContext; }

private:
    SkAutoTUnref<GrContext> fContext;
};

static void test_no_key(skiatest::Reporter* reporter) {
    Mock mock(10, 30000);
    GrContext* context = mock.context();
    GrResourceCache2* cache2 = mock.cache();

    // Create a bunch of resources with no keys
    TestResource* a = SkNEW_ARGS(TestResource, (context->getGpu()));
    TestResource* b = SkNEW_ARGS(TestResource, (context->getGpu()));
    TestResource* c = SkNEW_ARGS(TestResource, (context->getGpu()));
    TestResource* d = SkNEW_ARGS(TestResource, (context->getGpu()));
    a->setSize(11);
    b->setSize(12);
    c->setSize(13);
    d->setSize(14);

    REPORTER_ASSERT(reporter, 4 == TestResource::NumAlive());
    REPORTER_ASSERT(reporter, 4 == cache2->getResourceCount());
    REPORTER_ASSERT(reporter, a->gpuMemorySize() + b->gpuMemorySize() + c->gpuMemorySize() +
                              d->gpuMemorySize() == cache2->getResourceBytes());

    // Should be safe to purge without deleting the resources since we still have refs.
    cache2->purgeAllUnlocked();
    REPORTER_ASSERT(reporter, 4 == TestResource::NumAlive());

    // Since the resources have neither content nor scratch keys, delete immediately upon unref.

    a->unref();
    REPORTER_ASSERT(reporter, 3 == TestResource::NumAlive());
    REPORTER_ASSERT(reporter, 3 == cache2->getResourceCount());
    REPORTER_ASSERT(reporter, b->gpuMemorySize() + c->gpuMemorySize() + d->gpuMemorySize() ==
                              cache2->getResourceBytes());

    c->unref();
    REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());
    REPORTER_ASSERT(reporter, 2 == cache2->getResourceCount());
    REPORTER_ASSERT(reporter, b->gpuMemorySize() + d->gpuMemorySize() ==
                              cache2->getResourceBytes());

    d->unref();
    REPORTER_ASSERT(reporter, 1 == TestResource::NumAlive());
    REPORTER_ASSERT(reporter, 1 == cache2->getResourceCount());
    REPORTER_ASSERT(reporter, b->gpuMemorySize() == cache2->getResourceBytes());

    b->unref();
    REPORTER_ASSERT(reporter, 0 == TestResource::NumAlive());
    REPORTER_ASSERT(reporter, 0 == cache2->getResourceCount());
    REPORTER_ASSERT(reporter, 0 == cache2->getResourceBytes());
}

// Each integer passed as a template param creates a new domain.
template <int> static void make_content_key(GrContentKey* key, int data) {
    static GrContentKey::Domain d = GrContentKey::GenerateDomain();
    GrContentKey::Builder builder(key, d, 1);
    builder[0] = data;
}

static void test_budgeting(skiatest::Reporter* reporter) {
    Mock mock(10, 300);
    GrContext* context = mock.context();
    GrResourceCache2* cache2 = mock.cache();

    GrContentKey contentKey;
    make_content_key<0>(&contentKey, 0);

    // Create a scratch, a content, and a wrapped resource
    TestResource* scratch =
            TestResource::CreateScratchTestResource(context->getGpu(),
                                                    TestResource::kProperty2_SimulatedProperty);
    scratch->setSize(10);
    TestResource* content = SkNEW_ARGS(TestResource, (context->getGpu()));
    content->setSize(11);
    REPORTER_ASSERT(reporter, content->cacheAccess().setContentKey(contentKey));
    TestResource* wrapped = SkNEW_ARGS(TestResource,
                                       (context->getGpu(), GrGpuResource::kWrapped_LifeCycle));
    wrapped->setSize(12);
    TestResource* unbudgeted = SkNEW_ARGS(TestResource,
                                          (context->getGpu(), GrGpuResource::kUncached_LifeCycle));
    unbudgeted->setSize(13);

    // Make sure we can't add a content key to the wrapped resource
    GrContentKey contentKey2;
    make_content_key<0>(&contentKey2, 1);
    REPORTER_ASSERT(reporter, !wrapped->cacheAccess().setContentKey(contentKey2));
    REPORTER_ASSERT(reporter, NULL == cache2->findAndRefContentResource(contentKey2));

    // Make sure sizes are as we expect
    REPORTER_ASSERT(reporter, 4 == cache2->getResourceCount());
    REPORTER_ASSERT(reporter, scratch->gpuMemorySize() + content->gpuMemorySize() +
                              wrapped->gpuMemorySize() + unbudgeted->gpuMemorySize() ==
                              cache2->getResourceBytes());
    REPORTER_ASSERT(reporter, 2 == cache2->getBudgetedResourceCount());
    REPORTER_ASSERT(reporter, scratch->gpuMemorySize() + content->gpuMemorySize() ==
                              cache2->getBudgetedResourceBytes());

    // Our refs mean that the resources are non purgable.
    cache2->purgeAllUnlocked();
    REPORTER_ASSERT(reporter, 4 == cache2->getResourceCount());
    REPORTER_ASSERT(reporter, scratch->gpuMemorySize() + content->gpuMemorySize() +
                              wrapped->gpuMemorySize() + unbudgeted->gpuMemorySize() ==
                              cache2->getResourceBytes());
    REPORTER_ASSERT(reporter, 2 == cache2->getBudgetedResourceCount());
    REPORTER_ASSERT(reporter, scratch->gpuMemorySize() + content->gpuMemorySize() ==
                              cache2->getBudgetedResourceBytes());

    // Unreffing the wrapped resource should free it right away.
    wrapped->unref();
    REPORTER_ASSERT(reporter, 3 == cache2->getResourceCount());
    REPORTER_ASSERT(reporter, scratch->gpuMemorySize() + content->gpuMemorySize() +
                              unbudgeted->gpuMemorySize() == cache2->getResourceBytes());

    // Now try freeing the budgeted resources first
    wrapped = SkNEW_ARGS(TestResource, (context->getGpu(), GrGpuResource::kWrapped_LifeCycle));
    scratch->setSize(12);
    content->unref();
    cache2->purgeAllUnlocked();
    REPORTER_ASSERT(reporter, 3 == cache2->getResourceCount());
    REPORTER_ASSERT(reporter, scratch->gpuMemorySize() + wrapped->gpuMemorySize() +
                              unbudgeted->gpuMemorySize() == cache2->getResourceBytes());
    REPORTER_ASSERT(reporter, 1 == cache2->getBudgetedResourceCount());
    REPORTER_ASSERT(reporter, scratch->gpuMemorySize() == cache2->getBudgetedResourceBytes());

    scratch->unref();
    cache2->purgeAllUnlocked();
    REPORTER_ASSERT(reporter, 2 == cache2->getResourceCount());
    REPORTER_ASSERT(reporter, unbudgeted->gpuMemorySize() + wrapped->gpuMemorySize() ==
                              cache2->getResourceBytes());
    REPORTER_ASSERT(reporter, 0 == cache2->getBudgetedResourceCount());
    REPORTER_ASSERT(reporter, 0 == cache2->getBudgetedResourceBytes());

    wrapped->unref();
    REPORTER_ASSERT(reporter, 1 == cache2->getResourceCount());
    REPORTER_ASSERT(reporter, unbudgeted->gpuMemorySize() == cache2->getResourceBytes());
    REPORTER_ASSERT(reporter, 0 == cache2->getBudgetedResourceCount());
    REPORTER_ASSERT(reporter, 0 == cache2->getBudgetedResourceBytes());

    unbudgeted->unref();
    REPORTER_ASSERT(reporter, 0 == cache2->getResourceCount());
    REPORTER_ASSERT(reporter, 0 == cache2->getResourceBytes());
    REPORTER_ASSERT(reporter, 0 == cache2->getBudgetedResourceCount());
    REPORTER_ASSERT(reporter, 0 == cache2->getBudgetedResourceBytes());
}

static void test_unbudgeted(skiatest::Reporter* reporter) {
    Mock mock(10, 30000);
    GrContext* context = mock.context();
    GrResourceCache2* cache2 = mock.cache();

    GrContentKey contentKey;
    make_content_key<0>(&contentKey, 0);

    TestResource* scratch;
    TestResource* content;
    TestResource* wrapped;
    TestResource* unbudgeted;

    // A large uncached or wrapped resource shouldn't evict anything.
    scratch = TestResource::CreateScratchTestResource(context->getGpu(),
                                                      TestResource::kProperty2_SimulatedProperty);
    scratch->setSize(10);
    scratch->unref();
    REPORTER_ASSERT(reporter, 1 == cache2->getResourceCount());
    REPORTER_ASSERT(reporter, 10 == cache2->getResourceBytes());
    REPORTER_ASSERT(reporter, 1 == cache2->getBudgetedResourceCount());
    REPORTER_ASSERT(reporter, 10 == cache2->getBudgetedResourceBytes());

    content = SkNEW_ARGS(TestResource, (context->getGpu()));
    content->setSize(11);
    REPORTER_ASSERT(reporter, content->cacheAccess().setContentKey(contentKey));
    content->unref();
    REPORTER_ASSERT(reporter, 2 == cache2->getResourceCount());
    REPORTER_ASSERT(reporter, 21 == cache2->getResourceBytes());
    REPORTER_ASSERT(reporter, 2 == cache2->getBudgetedResourceCount());
    REPORTER_ASSERT(reporter, 21 == cache2->getBudgetedResourceBytes());

    size_t large = 2 * cache2->getResourceBytes();
    unbudgeted = SkNEW_ARGS(TestResource,
                            (context->getGpu(), large, GrGpuResource::kUncached_LifeCycle));
    REPORTER_ASSERT(reporter, 3 == cache2->getResourceCount());
    REPORTER_ASSERT(reporter, 21 + large == cache2->getResourceBytes());
    REPORTER_ASSERT(reporter, 2 == cache2->getBudgetedResourceCount());
    REPORTER_ASSERT(reporter, 21 == cache2->getBudgetedResourceBytes());

    unbudgeted->unref();
    REPORTER_ASSERT(reporter, 2 == cache2->getResourceCount());
    REPORTER_ASSERT(reporter, 21 == cache2->getResourceBytes());
    REPORTER_ASSERT(reporter, 2 == cache2->getBudgetedResourceCount());
    REPORTER_ASSERT(reporter, 21 == cache2->getBudgetedResourceBytes());

    wrapped = SkNEW_ARGS(TestResource,
                         (context->getGpu(), large, GrGpuResource::kWrapped_LifeCycle));
    REPORTER_ASSERT(reporter, 3 == cache2->getResourceCount());
    REPORTER_ASSERT(reporter, 21 + large == cache2->getResourceBytes());
    REPORTER_ASSERT(reporter, 2 == cache2->getBudgetedResourceCount());
    REPORTER_ASSERT(reporter, 21 == cache2->getBudgetedResourceBytes());

    wrapped->unref();
    REPORTER_ASSERT(reporter, 2 == cache2->getResourceCount());
    REPORTER_ASSERT(reporter, 21 == cache2->getResourceBytes());
    REPORTER_ASSERT(reporter, 2 == cache2->getBudgetedResourceCount());
    REPORTER_ASSERT(reporter, 21 == cache2->getBudgetedResourceBytes());

    cache2->purgeAllUnlocked();
    REPORTER_ASSERT(reporter, 0 == cache2->getResourceCount());
    REPORTER_ASSERT(reporter, 0 == cache2->getResourceBytes());
    REPORTER_ASSERT(reporter, 0 == cache2->getBudgetedResourceCount());
    REPORTER_ASSERT(reporter, 0 == cache2->getBudgetedResourceBytes());
}

static void test_unbudgeted_to_scratch(skiatest::Reporter* reporter) {
    Mock mock(10, 300);
    GrContext* context = mock.context();
    GrResourceCache2* cache2 = mock.cache();

    TestResource* resource =
        TestResource::CreateScratchTestResource(context->getGpu(),
                                                TestResource::kProperty1_SimulatedProperty, false);
    GrScratchKey key;
    TestResource::ComputeScratchKey(TestResource::kProperty1_SimulatedProperty, &key);

    size_t size = resource->gpuMemorySize();
    for (int i = 0; i < 2; ++i) {
        // Since this resource is unbudgeted, it should not be reachable as scratch.
        REPORTER_ASSERT(reporter, resource->cacheAccess().getScratchKey() == key);
        REPORTER_ASSERT(reporter, !resource->cacheAccess().isScratch());
        REPORTER_ASSERT(reporter, !resource->cacheAccess().isBudgeted());
        REPORTER_ASSERT(reporter, NULL == cache2->findAndRefScratchResource(key));
        REPORTER_ASSERT(reporter, 1 == cache2->getResourceCount());
        REPORTER_ASSERT(reporter, size == cache2->getResourceBytes());
        REPORTER_ASSERT(reporter, 0 == cache2->getBudgetedResourceCount());
        REPORTER_ASSERT(reporter, 0 == cache2->getBudgetedResourceBytes());

        // Once it is unrefed, it should become available as scratch.
        resource->unref();
        REPORTER_ASSERT(reporter, 1 == cache2->getResourceCount());
        REPORTER_ASSERT(reporter, size == cache2->getResourceBytes());
        REPORTER_ASSERT(reporter, 1 == cache2->getBudgetedResourceCount());
        REPORTER_ASSERT(reporter, size == cache2->getBudgetedResourceBytes());
        resource = static_cast<TestResource*>(cache2->findAndRefScratchResource(key));
        REPORTER_ASSERT(reporter, resource);
        REPORTER_ASSERT(reporter, resource->cacheAccess().getScratchKey() == key);
        REPORTER_ASSERT(reporter, resource->cacheAccess().isScratch());
        REPORTER_ASSERT(reporter, resource->cacheAccess().isBudgeted());

        if (0 == i) {
            // If made unbudgeted, it should return to original state: ref'ed and unbudgeted. Try 
            // the above tests again.
            resource->cacheAccess().makeUnbudgeted();
        } else {
            // After the second time around, try removing the scratch key
            resource->cacheAccess().removeScratchKey();
            REPORTER_ASSERT(reporter, 1 == cache2->getResourceCount());
            REPORTER_ASSERT(reporter, size == cache2->getResourceBytes());
            REPORTER_ASSERT(reporter, 1 == cache2->getBudgetedResourceCount());
            REPORTER_ASSERT(reporter, size == cache2->getBudgetedResourceBytes());
            REPORTER_ASSERT(reporter, !resource->cacheAccess().getScratchKey().isValid());
            REPORTER_ASSERT(reporter, !resource->cacheAccess().isScratch());
            REPORTER_ASSERT(reporter, resource->cacheAccess().isBudgeted());

            // now when it is unrefed it should die since it has no key.
            resource->unref();
            REPORTER_ASSERT(reporter, 0 == cache2->getResourceCount());
            REPORTER_ASSERT(reporter, 0 == cache2->getResourceBytes());
            REPORTER_ASSERT(reporter, 0 == cache2->getBudgetedResourceCount());
            REPORTER_ASSERT(reporter, 0 == cache2->getBudgetedResourceBytes());
        }
    }
}

static void test_duplicate_scratch_key(skiatest::Reporter* reporter) {
    Mock mock(5, 30000);
    GrContext* context = mock.context();
    GrResourceCache2* cache2 = mock.cache();

    // Create two resources that have the same scratch key.
    TestResource* a =
            TestResource::CreateScratchTestResource(context->getGpu(),
                                                    TestResource::kProperty2_SimulatedProperty);
    TestResource* b =
            TestResource::CreateScratchTestResource(context->getGpu(),
                                                    TestResource::kProperty2_SimulatedProperty);
    a->setSize(11);
    b->setSize(12);
    GrScratchKey scratchKey1;
    TestResource::ComputeScratchKey(TestResource::kProperty1_SimulatedProperty, &scratchKey1);
    // Check for negative case consistency. (leaks upon test failure.)
    REPORTER_ASSERT(reporter, NULL == cache2->findAndRefScratchResource(scratchKey1));

    GrScratchKey scratchKey;
    TestResource::ComputeScratchKey(TestResource::kProperty2_SimulatedProperty, &scratchKey);

    // Scratch resources are registered with GrResourceCache2 just by existing. There are 2.
    REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());
    SkDEBUGCODE(REPORTER_ASSERT(reporter, 2 == cache2->countScratchEntriesForKey(scratchKey));)
    REPORTER_ASSERT(reporter, 2 == cache2->getResourceCount());
    REPORTER_ASSERT(reporter, a->gpuMemorySize() + b->gpuMemorySize() ==
                              cache2->getResourceBytes());

    // Our refs mean that the resources are non purgable.
    cache2->purgeAllUnlocked();
    REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());
    REPORTER_ASSERT(reporter, 2 == cache2->getResourceCount());

    // Unref but don't purge
    a->unref();
    b->unref();
    REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());
    SkDEBUGCODE(REPORTER_ASSERT(reporter, 2 == cache2->countScratchEntriesForKey(scratchKey));)

    // Purge again. This time resources should be purgable.
    cache2->purgeAllUnlocked();
    REPORTER_ASSERT(reporter, 0 == TestResource::NumAlive());
    REPORTER_ASSERT(reporter, 0 == cache2->getResourceCount());
    SkDEBUGCODE(REPORTER_ASSERT(reporter, 0 == cache2->countScratchEntriesForKey(scratchKey));)
}

static void test_remove_scratch_key(skiatest::Reporter* reporter) {
    Mock mock(5, 30000);
    GrContext* context = mock.context();
    GrResourceCache2* cache2 = mock.cache();

    // Create two resources that have the same scratch key.
    TestResource* a =
            TestResource::CreateScratchTestResource(context->getGpu(),
                                                    TestResource::kProperty2_SimulatedProperty);
    TestResource* b =
            TestResource::CreateScratchTestResource(context->getGpu(),
                                                    TestResource::kProperty2_SimulatedProperty);
    a->unref();
    b->unref();


    GrScratchKey scratchKey;
    // Ensure that scratch key lookup is correct for negative case.
    TestResource::ComputeScratchKey(TestResource::kProperty1_SimulatedProperty, &scratchKey);
    // (following leaks upon test failure).
    REPORTER_ASSERT(reporter, cache2->findAndRefScratchResource(scratchKey) == NULL);

    // Scratch resources are registered with GrResourceCache2 just by existing. There are 2.
    TestResource::ComputeScratchKey(TestResource::kProperty2_SimulatedProperty, &scratchKey);
    REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());
    SkDEBUGCODE(REPORTER_ASSERT(reporter, 2 == cache2->countScratchEntriesForKey(scratchKey));)
    REPORTER_ASSERT(reporter, 2 == cache2->getResourceCount());

    // Find the first resource and remove its scratch key
    GrGpuResource* find;
    find = cache2->findAndRefScratchResource(scratchKey);
    find->cacheAccess().removeScratchKey();
    // It's still alive, but not cached by scratch key anymore
    REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());
    SkDEBUGCODE(REPORTER_ASSERT(reporter, 1 == cache2->countScratchEntriesForKey(scratchKey));)
    REPORTER_ASSERT(reporter, 2 == cache2->getResourceCount());

    // The cache should immediately delete it when it's unrefed since it isn't accessible.
    find->unref();
    REPORTER_ASSERT(reporter, 1 == TestResource::NumAlive());
    SkDEBUGCODE(REPORTER_ASSERT(reporter, 1 == cache2->countScratchEntriesForKey(scratchKey));)
    REPORTER_ASSERT(reporter, 1 == cache2->getResourceCount());

    // Repeat for the second resource.
    find = cache2->findAndRefScratchResource(scratchKey);
    find->cacheAccess().removeScratchKey();
    REPORTER_ASSERT(reporter, 1 == TestResource::NumAlive());
    SkDEBUGCODE(REPORTER_ASSERT(reporter, 0 == cache2->countScratchEntriesForKey(scratchKey));)
    REPORTER_ASSERT(reporter, 1 == cache2->getResourceCount());

    // Should be able to call this multiple times with no problem.
    find->cacheAccess().removeScratchKey();
    REPORTER_ASSERT(reporter, 1 == TestResource::NumAlive());
    SkDEBUGCODE(REPORTER_ASSERT(reporter, 0 == cache2->countScratchEntriesForKey(scratchKey));)
    REPORTER_ASSERT(reporter, 1 == cache2->getResourceCount());

    find->unref();
    REPORTER_ASSERT(reporter, 0 == TestResource::NumAlive());
    SkDEBUGCODE(REPORTER_ASSERT(reporter, 0 == cache2->countScratchEntriesForKey(scratchKey));)
    REPORTER_ASSERT(reporter, 0 == cache2->getResourceCount());
}

static void test_scratch_key_consistency(skiatest::Reporter* reporter) {
    Mock mock(5, 30000);
    GrContext* context = mock.context();
    GrResourceCache2* cache2 = mock.cache();

    // Create two resources that have the same scratch key.
    TestResource* a =
            TestResource::CreateScratchTestResource(context->getGpu(),
                                                    TestResource::kProperty2_SimulatedProperty);
    TestResource* b =
            TestResource::CreateScratchTestResource(context->getGpu(),
                                                    TestResource::kProperty2_SimulatedProperty);
    a->unref();
    b->unref();

    GrScratchKey scratchKey;
    // Ensure that scratch key comparison and assignment is consistent.
    GrScratchKey scratchKey1;
    TestResource::ComputeScratchKey(TestResource::kProperty1_SimulatedProperty, &scratchKey1);
    GrScratchKey scratchKey2;
    TestResource::ComputeScratchKey(TestResource::kProperty2_SimulatedProperty, &scratchKey2);
    REPORTER_ASSERT(reporter, scratchKey1.size() == TestResource::ExpectedScratchKeySize());
    REPORTER_ASSERT(reporter, scratchKey1 != scratchKey2);
    REPORTER_ASSERT(reporter, scratchKey2 != scratchKey1);
    scratchKey = scratchKey1;
    REPORTER_ASSERT(reporter, scratchKey.size() == TestResource::ExpectedScratchKeySize());
    REPORTER_ASSERT(reporter, scratchKey1 == scratchKey);
    REPORTER_ASSERT(reporter, scratchKey == scratchKey1);
    REPORTER_ASSERT(reporter, scratchKey2 != scratchKey);
    REPORTER_ASSERT(reporter, scratchKey != scratchKey2);
    scratchKey = scratchKey2;
    REPORTER_ASSERT(reporter, scratchKey.size() == TestResource::ExpectedScratchKeySize());
    REPORTER_ASSERT(reporter, scratchKey1 != scratchKey);
    REPORTER_ASSERT(reporter, scratchKey != scratchKey1);
    REPORTER_ASSERT(reporter, scratchKey2 == scratchKey);
    REPORTER_ASSERT(reporter, scratchKey == scratchKey2);

    // Ensure that scratch key lookup is correct for negative case.
    TestResource::ComputeScratchKey(TestResource::kProperty1_SimulatedProperty, &scratchKey);
    // (following leaks upon test failure).
    REPORTER_ASSERT(reporter, cache2->findAndRefScratchResource(scratchKey) == NULL);

    // Find the first resource with a scratch key and a copy of a scratch key.
    TestResource::ComputeScratchKey(TestResource::kProperty2_SimulatedProperty, &scratchKey);
    GrGpuResource* find = cache2->findAndRefScratchResource(scratchKey);
    REPORTER_ASSERT(reporter, find != NULL);
    find->unref();

    scratchKey2 = scratchKey;
    find = cache2->findAndRefScratchResource(scratchKey2);
    REPORTER_ASSERT(reporter, find != NULL);
    REPORTER_ASSERT(reporter, find == a || find == b);

    GrGpuResource* find2 = cache2->findAndRefScratchResource(scratchKey2);
    REPORTER_ASSERT(reporter, find2 != NULL);
    REPORTER_ASSERT(reporter, find2 == a || find2 == b);
    REPORTER_ASSERT(reporter, find2 != find);
    find2->unref();
    find->unref();
}

static void test_duplicate_content_key(skiatest::Reporter* reporter) {
    Mock mock(5, 30000);
    GrContext* context = mock.context();
    GrResourceCache2* cache2 = mock.cache();

    GrContentKey key;
    make_content_key<0>(&key, 0);
    
    // Create two resources that we will attempt to register with the same content key.
    TestResource* a = SkNEW_ARGS(TestResource, (context->getGpu()));
    TestResource* b = SkNEW_ARGS(TestResource, (context->getGpu()));
    a->setSize(11);
    b->setSize(12);
    
    // Can't set the same content key on two resources.
    REPORTER_ASSERT(reporter, a->cacheAccess().setContentKey(key));
    REPORTER_ASSERT(reporter, !b->cacheAccess().setContentKey(key));

    // Still have two resources because b is still reffed.
    REPORTER_ASSERT(reporter, 2 == cache2->getResourceCount());
    REPORTER_ASSERT(reporter, a->gpuMemorySize() + b->gpuMemorySize() ==
                              cache2->getResourceBytes());
    REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());

    b->unref();
    // Now b should be gone.
    REPORTER_ASSERT(reporter, 1 == cache2->getResourceCount());
    REPORTER_ASSERT(reporter, a->gpuMemorySize() == cache2->getResourceBytes());
    REPORTER_ASSERT(reporter, 1 == TestResource::NumAlive());

    cache2->purgeAllUnlocked();
    REPORTER_ASSERT(reporter, 1 == cache2->getResourceCount());
    REPORTER_ASSERT(reporter, a->gpuMemorySize() == cache2->getResourceBytes());
    REPORTER_ASSERT(reporter, 1 == TestResource::NumAlive());

    // Drop the ref on a but it isn't immediately purged as it still has a valid scratch key.
    a->unref();
    REPORTER_ASSERT(reporter, 1 == cache2->getResourceCount());
    REPORTER_ASSERT(reporter, a->gpuMemorySize() == cache2->getResourceBytes());
    REPORTER_ASSERT(reporter, 1 == TestResource::NumAlive());

    cache2->purgeAllUnlocked();
    REPORTER_ASSERT(reporter, 0 == cache2->getResourceCount());
    REPORTER_ASSERT(reporter, 0 == cache2->getResourceBytes());
    REPORTER_ASSERT(reporter, 0 == TestResource::NumAlive());
}

static void test_purge_invalidated(skiatest::Reporter* reporter) {
    Mock mock(5, 30000);
    GrContext* context = mock.context();
    GrResourceCache2* cache2 = mock.cache();

    GrContentKey key1, key2, key3;
    make_content_key<0>(&key1, 1);
    make_content_key<0>(&key2, 2);
    make_content_key<0>(&key3, 3);
    
    // Add three resources to the cache.
    TestResource* a = SkNEW_ARGS(TestResource, (context->getGpu()));
    TestResource* b = SkNEW_ARGS(TestResource, (context->getGpu()));
    TestResource* c = SkNEW_ARGS(TestResource, (context->getGpu()));
    a->cacheAccess().setContentKey(key1);
    b->cacheAccess().setContentKey(key2);
    c->cacheAccess().setContentKey(key3);
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
#if 0 // Disabled until reimplemented in GrResourceCache2.
    cache2->purgeAsNeeded();
    REPORTER_ASSERT(reporter, 1 == TestResource::NumAlive());
    REPORTER_ASSERT(reporter, !cache2->hasContentKey(key1));
    REPORTER_ASSERT(reporter, !cache2->hasContentKey(key2));
    REPORTER_ASSERT(reporter, cache2->hasContentKey(key3));
#endif

    // Invalidate the third.
    const GrResourceInvalidatedMessage msg3 = { key3 };
    SkMessageBus<GrResourceInvalidatedMessage>::Post(msg3);
#if 0 // Disabled until reimplemented in GrResourceCache2.
    cache2->purgeAsNeeded();
    REPORTER_ASSERT(reporter, 0 == TestResource::NumAlive());
    REPORTER_ASSERT(reporter, !cache2->hasContentKey(key3));
#endif

    cache2->purgeAllUnlocked();
    REPORTER_ASSERT(reporter, 0 == TestResource::NumAlive());
    REPORTER_ASSERT(reporter, 0 == cache2->getResourceCount());
    REPORTER_ASSERT(reporter, 0 == cache2->getResourceBytes());
}

static void test_cache_chained_purge(skiatest::Reporter* reporter) {
    Mock mock(3, 30000);
    GrContext* context = mock.context();
    GrResourceCache2* cache2 = mock.cache();

    GrContentKey key1, key2;
    make_content_key<0>(&key1, 1);
    make_content_key<0>(&key2, 2);


    TestResource* a = SkNEW_ARGS(TestResource, (context->getGpu()));
    TestResource* b = SkNEW_ARGS(TestResource, (context->getGpu()));
    a->cacheAccess().setContentKey(key1);
    b->cacheAccess().setContentKey(key2);

    // Make a cycle
    a->setUnrefWhenDestroyed(b);
    b->setUnrefWhenDestroyed(a);

    REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());

    a->unref();
    b->unref();

    REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());

    cache2->purgeAllUnlocked();
    REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());

    // Break the cycle
    a->setUnrefWhenDestroyed(NULL);
    REPORTER_ASSERT(reporter, 2 == TestResource::NumAlive());

    cache2->purgeAllUnlocked();
    REPORTER_ASSERT(reporter, 0 == TestResource::NumAlive());
}

static void test_resource_size_changed(skiatest::Reporter* reporter) {
    GrContentKey key1, key2;
    make_content_key<0>(&key1, 1);
    make_content_key<0>(&key2, 2);

    // Test changing resources sizes (both increase & decrease).
    {
        Mock mock(3, 30000);
        GrContext* context = mock.context();
        GrResourceCache2* cache2 = mock.cache();

        TestResource* a = SkNEW_ARGS(TestResource, (context->getGpu()));
        a->cacheAccess().setContentKey(key1);
        a->unref();

        TestResource* b = SkNEW_ARGS(TestResource, (context->getGpu()));
        b->cacheAccess().setContentKey(key2);
        b->unref();

        REPORTER_ASSERT(reporter, 200 == cache2->getResourceBytes());
        REPORTER_ASSERT(reporter, 2 == cache2->getResourceCount());
        {
            SkAutoTUnref<TestResource> find2(static_cast<TestResource*>(cache2->findAndRefContentResource(key2)));
            find2->setSize(200);
            SkAutoTUnref<TestResource> find1(static_cast<TestResource*>(cache2->findAndRefContentResource(key1)));
            find1->setSize(50);
        }

        REPORTER_ASSERT(reporter, 250 == cache2->getResourceBytes());
        REPORTER_ASSERT(reporter, 2 == cache2->getResourceCount());
    }

    // Test increasing a resources size beyond the cache budget.
    {
        Mock mock(2, 300);
        GrContext* context = mock.context();
        GrResourceCache2* cache2 = mock.cache();

        TestResource* a = SkNEW_ARGS(TestResource, (context->getGpu()));
        a->setSize(100);
        a->cacheAccess().setContentKey(key1);
        a->unref();

        TestResource* b = SkNEW_ARGS(TestResource, (context->getGpu()));
        b->setSize(100);
        b->cacheAccess().setContentKey(key2);
        b->unref();

        REPORTER_ASSERT(reporter, 200 == cache2->getResourceBytes());
        REPORTER_ASSERT(reporter, 2 == cache2->getResourceCount());

        {
            SkAutoTUnref<TestResource> find2(static_cast<TestResource*>(cache2->findAndRefContentResource(key2)));
            find2->setSize(201);
        }
        REPORTER_ASSERT(reporter, !cache2->hasContentKey(key1));

        REPORTER_ASSERT(reporter, 201 == cache2->getResourceBytes());
        REPORTER_ASSERT(reporter, 1 == cache2->getResourceCount());
    }
}

static void test_large_resource_count(skiatest::Reporter* reporter) {
    // Set the cache size to double the resource count because we're going to create 2x that number
    // resources, using two different key domains. Add a little slop to the bytes because we resize
    // down to 1 byte after creating the resource.
    static const int kResourceCnt = 2000;

    Mock mock(2 * kResourceCnt, 2 * kResourceCnt + 1000);
    GrContext* context = mock.context();
    GrResourceCache2* cache2 = mock.cache();

    for (int i = 0; i < kResourceCnt; ++i) {
        GrContentKey key1, key2;
        make_content_key<1>(&key1, i);
        make_content_key<2>(&key2, i);

        TestResource* resource;

        resource = SkNEW_ARGS(TestResource, (context->getGpu()));
        resource->cacheAccess().setContentKey(key1);
        resource->setSize(1);
        resource->unref();

        resource = SkNEW_ARGS(TestResource, (context->getGpu()));
        resource->cacheAccess().setContentKey(key2);
        resource->setSize(1);
        resource->unref();
    }

    REPORTER_ASSERT(reporter, TestResource::NumAlive() == 2 * kResourceCnt);
    REPORTER_ASSERT(reporter, cache2->getBudgetedResourceBytes() == 2 * kResourceCnt);
    REPORTER_ASSERT(reporter, cache2->getBudgetedResourceCount() == 2 * kResourceCnt);
    REPORTER_ASSERT(reporter, cache2->getResourceBytes() == 2 * kResourceCnt);
    REPORTER_ASSERT(reporter, cache2->getResourceCount() == 2 * kResourceCnt);
    for (int i = 0; i < kResourceCnt; ++i) {
        GrContentKey key1, key2;
        make_content_key<1>(&key1, i);
        make_content_key<2>(&key2, i);

        REPORTER_ASSERT(reporter, cache2->hasContentKey(key1));
        REPORTER_ASSERT(reporter, cache2->hasContentKey(key2));
    }

    cache2->purgeAllUnlocked();
    REPORTER_ASSERT(reporter, TestResource::NumAlive() == 0);
    REPORTER_ASSERT(reporter, cache2->getBudgetedResourceBytes() == 0);
    REPORTER_ASSERT(reporter, cache2->getBudgetedResourceCount() == 0);
    REPORTER_ASSERT(reporter, cache2->getResourceBytes() == 0);
    REPORTER_ASSERT(reporter, cache2->getResourceCount() == 0);

    for (int i = 0; i < kResourceCnt; ++i) {
        GrContentKey key1, key2;
        make_content_key<1>(&key1, i);
        make_content_key<2>(&key2, i);

        REPORTER_ASSERT(reporter, !cache2->hasContentKey(key1));
        REPORTER_ASSERT(reporter, !cache2->hasContentKey(key2));
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
        SkAutoTUnref<SkSurface> surface(SkSurface::NewRenderTarget(context,
                                                                   SkSurface::kNo_Budgeted, info));
        test_cache(reporter, context, surface->getCanvas());
    }

    // The below tests create their own mock contexts.
    test_no_key(reporter);
    test_budgeting(reporter);
    test_unbudgeted(reporter);
    test_unbudgeted_to_scratch(reporter);
    test_duplicate_content_key(reporter);
    test_duplicate_scratch_key(reporter);
    test_remove_scratch_key(reporter);
    test_scratch_key_consistency(reporter);
    test_purge_invalidated(reporter);
    test_cache_chained_purge(reporter);
    test_resource_size_changed(reporter);
    test_large_resource_count(reporter);
}

#endif
