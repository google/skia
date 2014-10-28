
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#if SK_SUPPORT_GPU

#include "Benchmark.h"
#include "GrGpuResource.h"
#include "GrContext.h"
#include "GrGpu.h"
#include "GrResourceCache.h"
#include "GrStencilBuffer.h"
#include "GrTexture.h"
#include "GrTexturePriv.h"
#include "SkCanvas.h"

enum {
    CACHE_SIZE_COUNT = 2048,
    CACHE_SIZE_BYTES = 2 * 1024 * 1024,
};

class StencilResource : public GrGpuResource {
public:
    SK_DECLARE_INST_COUNT(StencilResource);
    StencilResource(GrGpu* gpu, int id)
        : INHERITED(gpu, false)
        , fID(id) {
        this->registerWithCache();
    }

    virtual ~StencilResource() { this->release(); }

    virtual size_t gpuMemorySize() const SK_OVERRIDE {
        return 100 + ((fID % 1 == 0) ? -5 : 6);
    }

    static GrResourceKey ComputeKey(int width, int height, int sampleCnt) {
        return GrStencilBuffer::ComputeKey(width, height, sampleCnt);
    }

    int fID;

private:
    typedef GrGpuResource INHERITED;
};

class TextureResource : public GrGpuResource {
public:
    SK_DECLARE_INST_COUNT(TextureResource);
    TextureResource(GrGpu* gpu, int id)
        : INHERITED(gpu, false)
        , fID(id) {
        this->registerWithCache();
    }

    virtual ~TextureResource() { this->release(); }

    virtual size_t gpuMemorySize() const SK_OVERRIDE {
        return 100 + ((fID % 1 == 0) ? -40 : 33);
    }

    static GrResourceKey ComputeKey(const GrSurfaceDesc& desc) {
        GrCacheID::Key key;
        memset(&key, 0, sizeof(key));
        key.fData32[0] = (desc.fWidth) | (desc.fHeight << 16);
        key.fData32[1] = desc.fConfig | desc.fSampleCnt << 16;
        key.fData32[2] = desc.fFlags;
        static int gType = GrResourceKey::GenerateResourceType();
        static int gDomain = GrCacheID::GenerateDomain();
        return GrResourceKey(GrCacheID(gDomain, key), gType, 0);
    }

    int fID;

private:
    typedef GrGpuResource INHERITED;
};

static void get_stencil(int i, int* w, int* h, int* s) {
    *w = i % 1024;
    *h = i * 2 % 1024;
    *s = i % 1 == 0 ? 0 : 4;
}

static void get_texture_desc(int i, GrSurfaceDesc* desc) {
    desc->fFlags = kRenderTarget_GrSurfaceFlag |
        kNoStencil_GrSurfaceFlag;
    desc->fWidth  = i % 1024;
    desc->fHeight = i * 2 % 1024;
    desc->fConfig = static_cast<GrPixelConfig>(i % (kLast_GrPixelConfig + 1));
    desc->fSampleCnt = i % 1 == 0 ? 0 : 4;
}

static void populate_cache(GrResourceCache* cache, GrGpu* gpu, int resourceCount) {
    for (int i = 0; i < resourceCount; ++i) {
        int w, h, s;
        get_stencil(i, &w, &h, &s);
        GrResourceKey key = GrStencilBuffer::ComputeKey(w, h, s);
        GrGpuResource* resource = SkNEW_ARGS(StencilResource, (gpu, i));
        cache->purgeAsNeeded(1, resource->gpuMemorySize());
        cache->addResource(key, resource);
        resource->unref();
    }

    for (int i = 0; i < resourceCount; ++i) {
        GrSurfaceDesc desc;
        get_texture_desc(i, &desc);
        GrResourceKey key =  TextureResource::ComputeKey(desc);
        GrGpuResource* resource = SkNEW_ARGS(TextureResource, (gpu, i));
        cache->purgeAsNeeded(1, resource->gpuMemorySize());
        cache->addResource(key, resource);
        resource->unref();
    }
}

static void check_cache_contents_or_die(GrResourceCache* cache, int k) {
    // Benchmark find calls that succeed.
    {
        GrSurfaceDesc desc;
        get_texture_desc(k, &desc);
        GrResourceKey key = TextureResource::ComputeKey(desc);
        GrGpuResource* item = cache->find(key);
        if (NULL == item) {
            SkFAIL("cache add does not work as expected");
            return;
        }
        if (static_cast<TextureResource*>(item)->fID != k) {
            SkFAIL("cache add does not work as expected");
            return;
        }
    }
    {
        int w, h, s;
        get_stencil(k, &w, &h, &s);
        GrResourceKey key = StencilResource::ComputeKey(w, h, s);
        GrGpuResource* item = cache->find(key);
        if (NULL == item) {
            SkFAIL("cache add does not work as expected");
            return;
        }
        if (static_cast<TextureResource*>(item)->fID != k) {
            SkFAIL("cache add does not work as expected");
            return;
        }
    }

    // Benchmark also find calls that always fail.
    {
        GrSurfaceDesc desc;
        get_texture_desc(k, &desc);
        desc.fHeight |= 1;
        GrResourceKey key = TextureResource::ComputeKey(desc);
        GrGpuResource* item = cache->find(key);
        if (item) {
            SkFAIL("cache add does not work as expected");
            return;
        }
    }
    {
        int w, h, s;
        get_stencil(k, &w, &h, &s);
        h |= 1;
        GrResourceKey key = StencilResource::ComputeKey(w, h, s);
        GrGpuResource* item = cache->find(key);
        if (item) {
            SkFAIL("cache add does not work as expected");
            return;
        }
    }
}

class GrResourceCacheBenchAdd : public Benchmark {
    enum {
        RESOURCE_COUNT = CACHE_SIZE_COUNT / 2,
        DUPLICATE_COUNT = CACHE_SIZE_COUNT / 4,
    };

public:
    virtual bool isSuitableFor(Backend backend) SK_OVERRIDE {
        return backend == kGPU_Backend;
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return "grresourcecache_add";
    }

    virtual void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        GrGpu* gpu = canvas->getGrContext()->getGpu();

        for (int i = 0; i < loops; ++i) {
            GrResourceCache cache(gpu->caps(), CACHE_SIZE_COUNT, CACHE_SIZE_BYTES);
            populate_cache(&cache, gpu, DUPLICATE_COUNT);
            populate_cache(&cache, gpu, RESOURCE_COUNT);

            // Check that cache works.
            for (int k = 0; k < RESOURCE_COUNT; k += 33) {
                check_cache_contents_or_die(&cache, k);
            }
            cache.purgeAllUnlocked();
        }
    }

private:
    typedef Benchmark INHERITED;
};

class GrResourceCacheBenchFind : public Benchmark {
    enum {
        RESOURCE_COUNT = (CACHE_SIZE_COUNT / 2) - 100,
        DUPLICATE_COUNT = 100
    };

public:
    virtual bool isSuitableFor(Backend backend) SK_OVERRIDE {
        return backend == kGPU_Backend;
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return "grresourcecache_find";
    }

    virtual void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        GrGpu* gpu = canvas->getGrContext()->getGpu();
        GrResourceCache cache(gpu->caps(), CACHE_SIZE_COUNT, CACHE_SIZE_BYTES);
        populate_cache(&cache, gpu, DUPLICATE_COUNT);
        populate_cache(&cache, gpu, RESOURCE_COUNT);

        for (int i = 0; i < loops; ++i) {
            for (int k = 0; k < RESOURCE_COUNT; ++k) {
                check_cache_contents_or_die(&cache, k);
            }
        }
    }

private:
    typedef Benchmark INHERITED;
};

DEF_BENCH( return new GrResourceCacheBenchAdd(); )
DEF_BENCH( return new GrResourceCacheBenchFind(); )

#endif
