
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#if SK_SUPPORT_GPU

#include "GrContext.h"
#include "GrResource.h"
#include "GrResourceCache.h"
#include "GrStencilBuffer.h"
#include "GrTexture.h"
#include "SkBenchmark.h"
#include "SkCanvas.h"

enum {
    CACHE_SIZE_COUNT = 2048,
    CACHE_SIZE_BYTES = 2 * 1024 * 1024,
};

class StencilResource : public GrResource {
public:
    SK_DECLARE_INST_COUNT(StencilResource);
    StencilResource(GrGpu* gpu, int id)
        : INHERITED(gpu, false),
          fID(id) {
    }
    ~StencilResource() {
        this->release();
    }

    virtual size_t sizeInBytes() const SK_OVERRIDE {
        return 100 + ((fID % 1 == 0) ? -5 : 6);
    }

    static GrResourceKey ComputeKey(int width, int height, int sampleCnt) {
        return GrStencilBuffer::ComputeKey(width, height, sampleCnt);
    }

    int fID;

private:
    typedef GrResource INHERITED;
};

class TextureResource : public GrResource {
public:
    SK_DECLARE_INST_COUNT(TextureResource);
    TextureResource(GrGpu* gpu, int id)
        : INHERITED(gpu, false),
          fID(id) {
    }
    ~TextureResource() {
        this->release();
    }

    virtual size_t sizeInBytes() const SK_OVERRIDE {
        return 100 + ((fID % 1 == 0) ? -40 : 33);
    }

    static GrResourceKey ComputeKey(const GrTextureDesc& desc) {
        return GrTexture::ComputeScratchKey(desc);
    }

    int fID;

private:
    typedef GrResource INHERITED;
};

static void get_stencil(int i, int* w, int* h, int* s) {
    *w = i % 1024;
    *h = i * 2 % 1024;
    *s = i % 1 == 0 ? 0 : 4;
}

static void get_texture_desc(int i, GrTextureDesc* desc) {
    desc->fFlags = kRenderTarget_GrTextureFlagBit |
        kNoStencil_GrTextureFlagBit;
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
        GrResource* resource = SkNEW_ARGS(StencilResource, (gpu, i));
        cache->purgeAsNeeded(1, resource->sizeInBytes());
        cache->addResource(key, resource);
        resource->unref();
    }

    for (int i = 0; i < resourceCount; ++i) {
        GrTextureDesc desc;
        get_texture_desc(i, &desc);
        GrResourceKey key =  TextureResource::ComputeKey(desc);
        GrResource* resource = SkNEW_ARGS(TextureResource, (gpu, i));
        cache->purgeAsNeeded(1, resource->sizeInBytes());
        cache->addResource(key, resource);
        resource->unref();
    }
}

static void check_cache_contents_or_die(GrResourceCache* cache, int k) {
    // Benchmark find calls that succeed.
    {
        GrTextureDesc desc;
        get_texture_desc(k, &desc);
        GrResourceKey key = TextureResource::ComputeKey(desc);
        GrResource* item = cache->find(key);
        if (NULL == item) {
            GrCrash("cache add does not work as expected");
            return;
        }
        if (static_cast<TextureResource*>(item)->fID != k) {
            GrCrash("cache add does not work as expected");
            return;
        }
    }
    {
        int w, h, s;
        get_stencil(k, &w, &h, &s);
        GrResourceKey key = StencilResource::ComputeKey(w, h, s);
        GrResource* item = cache->find(key);
        if (NULL == item) {
            GrCrash("cache add does not work as expected");
            return;
        }
        if (static_cast<TextureResource*>(item)->fID != k) {
            GrCrash("cache add does not work as expected");
            return;
        }
    }

    // Benchmark also find calls that always fail.
    {
        GrTextureDesc desc;
        get_texture_desc(k, &desc);
        desc.fHeight |= 1;
        GrResourceKey key = TextureResource::ComputeKey(desc);
        GrResource* item = cache->find(key);
        if (NULL != item) {
            GrCrash("cache add does not work as expected");
            return;
        }
    }
    {
        int w, h, s;
        get_stencil(k, &w, &h, &s);
        h |= 1;
        GrResourceKey key = StencilResource::ComputeKey(w, h, s);
        GrResource* item = cache->find(key);
        if (NULL != item) {
            GrCrash("cache add does not work as expected");
            return;
        }
    }
}

class GrResourceCacheBenchAdd : public SkBenchmark {
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
            GrResourceCache cache(CACHE_SIZE_COUNT, CACHE_SIZE_BYTES);
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
    typedef SkBenchmark INHERITED;
};

class GrResourceCacheBenchFind : public SkBenchmark {
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
        GrResourceCache cache(CACHE_SIZE_COUNT, CACHE_SIZE_BYTES);
        populate_cache(&cache, gpu, DUPLICATE_COUNT);
        populate_cache(&cache, gpu, RESOURCE_COUNT);

        for (int i = 0; i < loops; ++i) {
            for (int k = 0; k < RESOURCE_COUNT; ++k) {
                check_cache_contents_or_die(&cache, k);
            }
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

DEF_BENCH( return new GrResourceCacheBenchAdd(); )
DEF_BENCH( return new GrResourceCacheBenchFind(); )

#endif
