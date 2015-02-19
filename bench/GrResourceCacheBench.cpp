
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"

#if SK_SUPPORT_GPU

#include "GrGpuResource.h"
#include "GrGpuResourcePriv.h"
#include "GrContext.h"
#include "GrGpu.h"
#include "GrResourceCache.h"
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

    static void ComputeKey(int i, GrUniqueKey* key) {
        static GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
        GrUniqueKey::Builder builder(key, kDomain, 1);
        builder[0] = i;
    }

private:
    size_t onGpuMemorySize() const SK_OVERRIDE { return 100; }

    typedef GrGpuResource INHERITED;
};

static void populate_cache(GrGpu* gpu, int resourceCount) {
    for (int i = 0; i < resourceCount; ++i) {
        GrUniqueKey key;
        BenchResource::ComputeKey(i, &key);
        GrGpuResource* resource = SkNEW_ARGS(BenchResource, (gpu));
        resource->resourcePriv().setUniqueKey(key);
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

        GrResourceCache* cache = context->getResourceCache();

        // Make sure the cache is empty.
        cache->purgeAllUnlocked();
        SkASSERT(0 == cache->getResourceCount() && 0 == cache->getResourceBytes());

        GrGpu* gpu = context->getGpu();

        for (int i = 0; i < loops; ++i) {
            populate_cache(gpu, CACHE_SIZE_COUNT);
            SkASSERT(CACHE_SIZE_COUNT == cache->getResourceCount());
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

        GrResourceCache* cache = fContext->getResourceCache();

        // Make sure the cache is empty.
        cache->purgeAllUnlocked();
        SkASSERT(0 == cache->getResourceCount() && 0 == cache->getResourceBytes());

        GrGpu* gpu = fContext->getGpu();

        populate_cache(gpu, CACHE_SIZE_COUNT);
    }

    void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        if (!fContext) {
            return;
        }
        GrResourceCache* cache = fContext->getResourceCache();
        SkASSERT(CACHE_SIZE_COUNT == cache->getResourceCount());
        for (int i = 0; i < loops; ++i) {
            for (int k = 0; k < CACHE_SIZE_COUNT; ++k) {
                GrUniqueKey key;
                BenchResource::ComputeKey(k, &key);
                SkAutoTUnref<GrGpuResource> resource(cache->findAndRefUniqueResource(key));
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
