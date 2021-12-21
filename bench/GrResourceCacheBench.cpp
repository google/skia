/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"

#include "include/core/SkCanvas.h"
#include "include/gpu/GrDirectContext.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrGpuResource.h"
#include "src/gpu/GrGpuResourcePriv.h"
#include "src/gpu/GrResourceCache.h"

enum {
    CACHE_SIZE_COUNT = 4096,
};

class BenchResource : public GrGpuResource {
public:
    BenchResource (GrGpu* gpu)
        : INHERITED(gpu) {
        this->registerWithCache(SkBudgeted::kYes);
    }

    static void ComputeKey(int i, int keyData32Count, skgpu::UniqueKey* key) {
        static skgpu::UniqueKey::Domain kDomain = skgpu::UniqueKey::GenerateDomain();
        skgpu::UniqueKey::Builder builder(key, kDomain, keyData32Count);
        for (int j = 0; j < keyData32Count; ++j) {
            builder[j] = i + j;
        }
    }

private:
    size_t onGpuMemorySize() const override { return 100; }
    const char* getResourceType() const override { return "bench"; }
    using INHERITED = GrGpuResource;
};

static void populate_cache(GrGpu* gpu, int resourceCount, int keyData32Count) {
    for (int i = 0; i < resourceCount; ++i) {
        skgpu::UniqueKey key;
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
        sk_sp<GrDirectContext> context(GrDirectContext::MakeMock(nullptr));
        if (nullptr == context) {
            return;
        }
        // Set the cache budget to be very large so no purging occurs.
        context->setResourceCacheLimits(CACHE_SIZE_COUNT, 1 << 30);

        GrResourceCache* cache = context->priv().getResourceCache();

        // Make sure the cache is empty.
        cache->purgeUnlockedResources();
        SkASSERT(0 == cache->getResourceCount() && 0 == cache->getResourceBytes());

        GrGpu* gpu = context->priv().getGpu();

        for (int i = 0; i < loops; ++i) {
            populate_cache(gpu, CACHE_SIZE_COUNT, fKeyData32Count);
            SkASSERT(CACHE_SIZE_COUNT == cache->getResourceCount());
        }
    }

private:
    SkString fFullName;
    int fKeyData32Count;
    using INHERITED = Benchmark;
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
        fContext = GrDirectContext::MakeMock(nullptr);
        if (!fContext) {
            return;
        }
        // Set the cache budget to be very large so no purging occurs.
        fContext->setResourceCacheLimits(CACHE_SIZE_COUNT, 1 << 30);

        GrResourceCache* cache = fContext->priv().getResourceCache();

        // Make sure the cache is empty.
        cache->purgeUnlockedResources();
        SkASSERT(0 == cache->getResourceCount() && 0 == cache->getResourceBytes());

        GrGpu* gpu = fContext->priv().getGpu();

        populate_cache(gpu, CACHE_SIZE_COUNT, fKeyData32Count);
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        if (!fContext) {
            return;
        }
        GrResourceCache* cache = fContext->priv().getResourceCache();
        SkASSERT(CACHE_SIZE_COUNT == cache->getResourceCount());
        for (int i = 0; i < loops; ++i) {
            for (int k = 0; k < CACHE_SIZE_COUNT; ++k) {
                skgpu::UniqueKey key;
                BenchResource::ComputeKey(k, fKeyData32Count, &key);
                sk_sp<GrGpuResource> resource(cache->findAndRefUniqueResource(key));
                SkASSERT(resource);
            }
        }
    }

private:
    sk_sp<GrDirectContext> fContext;
    SkString fFullName;
    int fKeyData32Count;
    using INHERITED = Benchmark;
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
