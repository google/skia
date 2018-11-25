/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"

#if SK_SUPPORT_GPU

#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrGpu.h"
#include "GrGpuResource.h"
#include "GrGpuResourcePriv.h"
#include "GrResourceCache.h"
#include "SkCanvas.h"

enum {
    CACHE_SIZE_COUNT = 4096,
};

class BenchResource : public GrGpuResource {
public:
    BenchResource (GrGpu* gpu)
        : INHERITED(gpu) {
        this->registerWithCache(SkBudgeted::kYes);
    }

    static void ComputeKey(int i, int keyData32Count, GrUniqueKey* key) {
        static GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
        GrUniqueKey::Builder builder(key, kDomain, keyData32Count);
        for (int j = 0; j < keyData32Count; ++j) {
            builder[j] = i + j;
        }
    }

private:
    size_t onGpuMemorySize() const override { return 100; }
    typedef GrGpuResource INHERITED;
};

static void populate_cache(GrGpu* gpu, int resourceCount, int keyData32Count) {
    for (int i = 0; i < resourceCount; ++i) {
        GrUniqueKey key;
        BenchResource::ComputeKey(i, keyData32Count, &key);
        GrGpuResource* resource = new BenchResource(gpu);
        resource->resourcePriv().setUniqueKey(key);
        resource->unref();
    }
}

class GrResourceCacheBenchAdd : public Benchmark {
public:
    GrResourceCacheBenchAdd(int keyData32Count)
        : fFullName("grresourcecache_add")
        , fKeyData32Count(keyData32Count) {
        if (keyData32Count > 1) {
            fFullName.appendf("_%d", fKeyData32Count);
        }
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }
protected:
    const char* onGetName() override {
        return fFullName.c_str();
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        sk_sp<GrContext> context(GrContext::MakeMock(nullptr));
        if (nullptr == context) {
            return;
        }
        // Set the cache budget to be very large so no purging occurs.
        context->setResourceCacheLimits(CACHE_SIZE_COUNT, 1 << 30);

        GrResourceCache* cache = context->contextPriv().getResourceCache();

        // Make sure the cache is empty.
        cache->purgeAllUnlocked();
        SkASSERT(0 == cache->getResourceCount() && 0 == cache->getResourceBytes());

        GrGpu* gpu = context->contextPriv().getGpu();

        for (int i = 0; i < loops; ++i) {
            populate_cache(gpu, CACHE_SIZE_COUNT, fKeyData32Count);
            SkASSERT(CACHE_SIZE_COUNT == cache->getResourceCount());
        }
    }

private:
    SkString fFullName;
    int fKeyData32Count;
    typedef Benchmark INHERITED;
};

class GrResourceCacheBenchFind : public Benchmark {
public:
    GrResourceCacheBenchFind(int keyData32Count)
        : fFullName("grresourcecache_find")
        , fKeyData32Count(keyData32Count) {
        if (keyData32Count > 1) {
            fFullName.appendf("_%d", fKeyData32Count);
        }
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }
protected:
    const char* onGetName() override {
        return fFullName.c_str();
    }

    void onDelayedSetup() override {
        fContext = GrContext::MakeMock(nullptr);
        if (!fContext) {
            return;
        }
        // Set the cache budget to be very large so no purging occurs.
        fContext->setResourceCacheLimits(CACHE_SIZE_COUNT, 1 << 30);

        GrResourceCache* cache = fContext->contextPriv().getResourceCache();

        // Make sure the cache is empty.
        cache->purgeAllUnlocked();
        SkASSERT(0 == cache->getResourceCount() && 0 == cache->getResourceBytes());

        GrGpu* gpu = fContext->contextPriv().getGpu();

        populate_cache(gpu, CACHE_SIZE_COUNT, fKeyData32Count);
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        if (!fContext) {
            return;
        }
        GrResourceCache* cache = fContext->contextPriv().getResourceCache();
        SkASSERT(CACHE_SIZE_COUNT == cache->getResourceCount());
        for (int i = 0; i < loops; ++i) {
            for (int k = 0; k < CACHE_SIZE_COUNT; ++k) {
                GrUniqueKey key;
                BenchResource::ComputeKey(k, fKeyData32Count, &key);
                sk_sp<GrGpuResource> resource(cache->findAndRefUniqueResource(key));
                SkASSERT(resource);
            }
        }
    }

private:
    sk_sp<GrContext> fContext;
    SkString fFullName;
    int fKeyData32Count;
    typedef Benchmark INHERITED;
};

DEF_BENCH( return new GrResourceCacheBenchAdd(1); )
#ifdef SK_RELEASE
// Only on release because on debug the SkTDynamicHash validation is too slow.
DEF_BENCH( return new GrResourceCacheBenchAdd(2); )
DEF_BENCH( return new GrResourceCacheBenchAdd(3); )
DEF_BENCH( return new GrResourceCacheBenchAdd(4); )
DEF_BENCH( return new GrResourceCacheBenchAdd(5); )
DEF_BENCH( return new GrResourceCacheBenchAdd(10); )
DEF_BENCH( return new GrResourceCacheBenchAdd(25); )
DEF_BENCH( return new GrResourceCacheBenchAdd(54); )
DEF_BENCH( return new GrResourceCacheBenchAdd(55); )
DEF_BENCH( return new GrResourceCacheBenchAdd(56); )
#endif

DEF_BENCH( return new GrResourceCacheBenchFind(1); )
#ifdef SK_RELEASE
DEF_BENCH( return new GrResourceCacheBenchFind(2); )
DEF_BENCH( return new GrResourceCacheBenchFind(3); )
DEF_BENCH( return new GrResourceCacheBenchFind(4); )
DEF_BENCH( return new GrResourceCacheBenchFind(5); )
DEF_BENCH( return new GrResourceCacheBenchFind(10); )
DEF_BENCH( return new GrResourceCacheBenchFind(25); )
DEF_BENCH( return new GrResourceCacheBenchFind(54); )
DEF_BENCH( return new GrResourceCacheBenchFind(55); )
DEF_BENCH( return new GrResourceCacheBenchFind(56); )
#endif

#endif
