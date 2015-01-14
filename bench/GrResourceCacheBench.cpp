
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"

#if SK_SUPPORT_GPU

#include "GrGpuResource.h"
#include "GrContext.h"
#include "GrGpu.h"
#include "GrResourceCache2.h"
#include "SkCanvas.h"

enum {
    CACHE_SIZE_COUNT = 4096,
};

class BenchResource : public GrGpuResource {
public:
    SK_DECLARE_INST_COUNT(BenchResource);
    BenchResource (GrGpu* gpu)
        : INHERITED(gpu, kCached_LifeCycle) {
        this->registerWithCache();
    }

    static GrResourceKey ComputeKey(int i) {
        GrCacheID::Key key;
        memset(&key, 0, sizeof(key));
        key.fData32[0] = i;
        static int gDomain = GrCacheID::GenerateDomain();
        return GrResourceKey(GrCacheID(gDomain, key), 0);
    }


private:
    size_t onGpuMemorySize() const SK_OVERRIDE { return 100; }

    typedef GrGpuResource INHERITED;
};

static void populate_cache(GrGpu* gpu, int resourceCount) {
    for (int i = 0; i < resourceCount; ++i) {
        GrResourceKey key = BenchResource::ComputeKey(i);
        GrGpuResource* resource = SkNEW_ARGS(BenchResource, (gpu));
        resource->cacheAccess().setContentKey(key);
        resource->unref();
    }
}

class GrResourceCacheBenchAdd : public Benchmark {
public:
    bool isSuitableFor(Backend backend) SK_OVERRIDE {
        return backend == kNonRendering_Backend;
    }

protected:
    const char* onGetName() SK_OVERRIDE {
        return "grresourcecache_add";
    }

    void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        SkAutoTUnref<GrContext> context(GrContext::CreateMockContext());
        if (NULL == context) {
            return;
        }
        // Set the cache budget to be very large so no purging occurs.
        context->setResourceCacheLimits(CACHE_SIZE_COUNT, 1 << 30);

        GrResourceCache2* cache2 = context->getResourceCache2();

        // Make sure the cache is empty.
        cache2->purgeAllUnlocked();
        SkASSERT(0 == cache2->getResourceCount() && 0 == cache2->getResourceBytes());

        GrGpu* gpu = context->getGpu();

        for (int i = 0; i < loops; ++i) {
            populate_cache(gpu, CACHE_SIZE_COUNT);
            SkASSERT(CACHE_SIZE_COUNT == cache2->getResourceCount());
        }
    }

private:
    typedef Benchmark INHERITED;
};

class GrResourceCacheBenchFind : public Benchmark {
public:
    bool isSuitableFor(Backend backend) SK_OVERRIDE {
        return backend == kNonRendering_Backend;
    }

protected:
    const char* onGetName() SK_OVERRIDE {
        return "grresourcecache_find";
    }

    void onPreDraw() SK_OVERRIDE {
        fContext.reset(GrContext::CreateMockContext());
        if (!fContext) {
            return;
        }
        // Set the cache budget to be very large so no purging occurs.
        fContext->setResourceCacheLimits(CACHE_SIZE_COUNT, 1 << 30);

        GrResourceCache2* cache2 = fContext->getResourceCache2();

        // Make sure the cache is empty.
        cache2->purgeAllUnlocked();
        SkASSERT(0 == cache2->getResourceCount() && 0 == cache2->getResourceBytes());

        GrGpu* gpu = fContext->getGpu();

        populate_cache(gpu, CACHE_SIZE_COUNT);
    }

    void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        if (!fContext) {
            return;
        }
        GrResourceCache2* cache2 = fContext->getResourceCache2();
        SkASSERT(CACHE_SIZE_COUNT == cache2->getResourceCount());
        for (int i = 0; i < loops; ++i) {
            for (int k = 0; k < CACHE_SIZE_COUNT; ++k) {
                GrResourceKey key = BenchResource::ComputeKey(k);
                SkAutoTUnref<GrGpuResource> resource(cache2->findAndRefContentResource(key));
                SkASSERT(resource);
            }
        }
    }

private:
    SkAutoTUnref<GrContext> fContext;
    typedef Benchmark INHERITED;
};

DEF_BENCH( return new GrResourceCacheBenchAdd(); )
DEF_BENCH( return new GrResourceCacheBenchFind(); )

#endif
