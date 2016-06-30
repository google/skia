/*
* Copyright 2014 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

// Disabling this test since it is for the layer hoister which is current disabled.
// The test fails when we add a discard to a newly created render target.
#if 0

#if SK_SUPPORT_GPU

#include "GrContext.h"
#include "GrLayerCache.h"
#include "GrResourceCache.h"
#include "SkPictureRecorder.h"
#include "Test.h"

class TestingAccess {
public:
    static int NumPlots() {
        return GrLayerCache::kNumPlotsX * GrLayerCache::kNumPlotsY;
    }
    static SkISize PlotSize() {
        return SkISize::Make(GrLayerCache::kAtlasTextureWidth / GrLayerCache::kNumPlotsX,
                             GrLayerCache::kAtlasTextureHeight / GrLayerCache::kNumPlotsY);
    }

    static GrTexture* GetBackingTexture(GrLayerCache* cache) {
        return cache->fAtlas->getTextureOrNull();
    }

    static int NumLayers(GrLayerCache* cache) {
        return cache->numLayers();
    }
    static void Purge(GrLayerCache* cache, uint32_t pictureID) {
        cache->purge(pictureID);
    }
    static int Uses(GrCachedLayer* layer) {
        return layer->uses();
    }
    static GrCachedLayer* Find(GrLayerCache* cache, uint32_t pictureID,
                               const SkMatrix& initialMat,
                               const int* key, int keySize) {
        return cache->findLayer(pictureID, initialMat, key, keySize);
    }
};

// Add several layers to the cache
static void create_layers(skiatest::Reporter* reporter,
                          GrLayerCache* cache,
                          const SkPicture& picture,
                          int numToAdd,
                          int idOffset) {

    for (int i = 0; i < numToAdd; ++i) {
        int key[1] = { idOffset+i+1 };
        GrCachedLayer* layer = cache->findLayerOrCreate(picture.uniqueID(),
                                                        idOffset+i+1, idOffset+i+2,
                                                        SkIRect::MakeEmpty(),
                                                        SkIRect::MakeEmpty(),
                                                        SkMatrix::I(),
                                                        key, 1,
                                                        nullptr);
        REPORTER_ASSERT(reporter, layer);
        GrCachedLayer* temp = TestingAccess::Find(cache, picture.uniqueID(), SkMatrix::I(),
                                                  key, 1);
        REPORTER_ASSERT(reporter, temp == layer);

        REPORTER_ASSERT(reporter, TestingAccess::NumLayers(cache) == idOffset + i + 1);

        REPORTER_ASSERT(reporter, picture.uniqueID() == layer->pictureID());
        REPORTER_ASSERT(reporter, layer->start() == idOffset + i + 1);
        REPORTER_ASSERT(reporter, layer->stop() == idOffset + i + 2);
        REPORTER_ASSERT(reporter, !layer->texture());
        REPORTER_ASSERT(reporter, !layer->paint());
        REPORTER_ASSERT(reporter, !layer->isAtlased());
    }
}

static void lock_layer(skiatest::Reporter* reporter,
                       GrLayerCache* cache,
                       GrCachedLayer* layer) {
    // Make each layer big enough to consume one whole plot in the atlas
    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = TestingAccess::PlotSize().fWidth;
    desc.fHeight = TestingAccess::PlotSize().fHeight;
    desc.fConfig = kSkia8888_GrPixelConfig;

    bool needsRerendering;
    bool inAtlas = cache->tryToAtlas(layer, desc, &needsRerendering);
    if (!inAtlas) {
        cache->lock(layer, desc, &needsRerendering);
    }
    REPORTER_ASSERT(reporter, needsRerendering);

    cache->lock(layer, desc, &needsRerendering);
    REPORTER_ASSERT(reporter, !needsRerendering);

    REPORTER_ASSERT(reporter, layer->texture());
    REPORTER_ASSERT(reporter, layer->locked());

    cache->addUse(layer);

    REPORTER_ASSERT(reporter, 1 == TestingAccess::Uses(layer));
}

// This test case exercises the public API of the GrLayerCache class.
// In particular it checks its interaction with the resource cache (w.r.t.
// locking & unlocking textures).
// TODO: need to add checks on VRAM usage!
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GpuLayerCache, reporter, ctxInfo) {
    // Add one more layer than can fit in the atlas
    static const int kInitialNumLayers = TestingAccess::NumPlots() + 1;

#if GR_CACHE_STATS
    GrResourceCache::Stats stats;
#endif

    sk_sp<SkPicture> picture;

    {
        SkPictureRecorder recorder;
        SkCanvas* c = recorder.beginRecording(1, 1);
        // Draw something, anything, to prevent an empty-picture optimization,
        // which is a singleton and never purged.
        c->drawRect(SkRect::MakeWH(1,1), SkPaint());
        picture = recorder.finishRecordingAsPicture();
    }

    GrResourceCache* resourceCache = ctxInfo.grContext()->getResourceCache();

    GrLayerCache cache(ctxInfo.grContext());

    create_layers(reporter, &cache, *picture, kInitialNumLayers, 0);

    for (int i = 0; i < kInitialNumLayers; ++i) {
        int key[1] = { i + 1 };
        GrCachedLayer* layer = TestingAccess::Find(&cache, picture->uniqueID(), SkMatrix::I(),
                                                   key, 1);
        REPORTER_ASSERT(reporter, layer);

        lock_layer(reporter, &cache, layer);

#if GR_CACHE_STATS
        resourceCache->getStats(&stats);
#endif

        // The first 4 layers should be in the atlas (and thus have non-empty rects)
        if (i < TestingAccess::NumPlots()) {
            REPORTER_ASSERT(reporter, layer->isAtlased());
#if GR_CACHE_STATS
            REPORTER_ASSERT(reporter, 1 == stats.fTotal);
#endif
        } else {
            // The 5th layer couldn't fit in the atlas
            REPORTER_ASSERT(reporter, !layer->isAtlased());
#if GR_CACHE_STATS
            REPORTER_ASSERT(reporter, 2 == stats.fTotal);
#endif
        }
    }

    // Unlock the textures
    for (int i = 0; i < kInitialNumLayers; ++i) {
        int key[1] = { i+1 };

        GrCachedLayer* layer = TestingAccess::Find(&cache, picture->uniqueID(), SkMatrix::I(),
                                                   key, 1);
        REPORTER_ASSERT(reporter, layer);
        cache.removeUse(layer);
    }

#if GR_CACHE_STATS
    resourceCache->getStats(&stats);
    REPORTER_ASSERT(reporter, 2 == stats.fTotal);
    // The floating layer is purgeable the cache is not
    REPORTER_ASSERT(reporter, 1 == stats.fNumPurgeable);
    REPORTER_ASSERT(reporter, 1 == stats.fNumNonPurgeable);
#endif

    for (int i = 0; i < kInitialNumLayers; ++i) {
        int key[1] = { i+1 };

        GrCachedLayer* layer = TestingAccess::Find(&cache, picture->uniqueID(), SkMatrix::I(),
                                                   key, 1);
        REPORTER_ASSERT(reporter, layer);

        // All the layers should be unlocked
        REPORTER_ASSERT(reporter, !layer->locked());

        // When hoisted layers aren't cached they are aggressively removed
        // from the atlas
#if GR_CACHE_HOISTED_LAYERS
        // The first 4 layers should still be in the atlas.
        if (i < 4) {
            REPORTER_ASSERT(reporter, layer->texture());
            REPORTER_ASSERT(reporter, layer->isAtlased());
        } else {
#endif
            // The final layer should not be atlased.
            REPORTER_ASSERT(reporter, !layer->texture());
            REPORTER_ASSERT(reporter, !layer->isAtlased());
#if GR_CACHE_HOISTED_LAYERS
        }
#endif
    }

    // Let go of the backing texture
    cache.end();
    REPORTER_ASSERT(reporter, nullptr == TestingAccess::GetBackingTexture(&cache));

#if GR_CACHE_STATS
    resourceCache->getStats(&stats);
    REPORTER_ASSERT(reporter, 2 == stats.fTotal);
    // Now both the floater and the atlas are purgeable
    REPORTER_ASSERT(reporter, 2 == stats.fNumPurgeable);
#endif

    // re-attach to the backing texture
    cache.begin();
    REPORTER_ASSERT(reporter, TestingAccess::GetBackingTexture(&cache));

#if GR_CACHE_STATS
    resourceCache->getStats(&stats);
    REPORTER_ASSERT(reporter, 2 == stats.fTotal);
    // The atlas is restored to being non-purgeable
    REPORTER_ASSERT(reporter, 1 == stats.fNumPurgeable);
    REPORTER_ASSERT(reporter, 1 == stats.fNumNonPurgeable);
#endif

    {
        int key[1] = { kInitialNumLayers+1 };

        // Add an additional layer. Since all the layers are unlocked this
        // will force out the first atlased layer
        create_layers(reporter, &cache, *picture, 1, kInitialNumLayers);
        GrCachedLayer* layer = TestingAccess::Find(&cache, picture->uniqueID(), SkMatrix::I(),
                                                   key, 1);
        REPORTER_ASSERT(reporter, layer);

        lock_layer(reporter, &cache, layer);
        cache.removeUse(layer);
    }

    for (int i = 0; i < kInitialNumLayers+1; ++i) {
        int key[1] = { i+1 };

        GrCachedLayer* layer = TestingAccess::Find(&cache, picture->uniqueID(), SkMatrix::I(),
                                                   key, 1);
#if GR_CACHE_HOISTED_LAYERS
        // 3 old layers plus the new one should be in the atlas.
        if (1 == i || 2 == i || 3 == i || 5 == i) {
            REPORTER_ASSERT(reporter, layer);
            REPORTER_ASSERT(reporter, !layer->locked());
            REPORTER_ASSERT(reporter, layer->texture());
            REPORTER_ASSERT(reporter, layer->isAtlased());
        } else if (4 == i) {
#endif
            // The one that was never atlased should still be around
            REPORTER_ASSERT(reporter, layer);

            REPORTER_ASSERT(reporter, !layer->texture());
            REPORTER_ASSERT(reporter, !layer->isAtlased());
#if GR_CACHE_HOISTED_LAYERS
        } else {
            // The one bumped out of the atlas (i.e., 0) should be gone
            REPORTER_ASSERT(reporter, nullptr == layer);
        }
#endif
    }

    //--------------------------------------------------------------------
    // Free them all SkGpuDevice-style. This will not free up the
    // atlas' texture but will eliminate all the layers.
    TestingAccess::Purge(&cache, picture->uniqueID());

    REPORTER_ASSERT(reporter, TestingAccess::NumLayers(&cache) == 0);

#if GR_CACHE_STATS
    resourceCache->getStats(&stats);
    REPORTER_ASSERT(reporter, 2 == stats.fTotal);
    // Atlas isn't purgeable
    REPORTER_ASSERT(reporter, 1 == stats.fNumPurgeable);
    REPORTER_ASSERT(reporter, 1 == stats.fNumNonPurgeable);
#endif

    //--------------------------------------------------------------------
    // Test out the GrContext-style purge. This should remove all the layers
    // and the atlas.
    // Re-create the layers
    create_layers(reporter, &cache, *picture, kInitialNumLayers, 0);

    // Free them again GrContext-style. This should free up everything.
    cache.freeAll();

    REPORTER_ASSERT(reporter, TestingAccess::NumLayers(&cache) == 0);

    REPORTER_ASSERT(reporter, nullptr == TestingAccess::GetBackingTexture(&cache));

#if GR_CACHE_STATS
    resourceCache->getStats(&stats);
    REPORTER_ASSERT(reporter, 2 == stats.fTotal);
    REPORTER_ASSERT(reporter, 2 == stats.fNumPurgeable);
#endif

    // Purge the resource cache ...
    resourceCache->purgeAllUnlocked();

#if GR_CACHE_STATS
    resourceCache->getStats(&stats);
    REPORTER_ASSERT(reporter, 0 == stats.fTotal);
#endif

    // and try to re-attach to the backing texture. This should fail
    cache.begin();
    REPORTER_ASSERT(reporter, nullptr == TestingAccess::GetBackingTexture(&cache));

    //--------------------------------------------------------------------
    // Test out the MessageBus-style purge. This will not free the atlas
    // but should eliminate the free-floating layers.
    create_layers(reporter, &cache, *picture, kInitialNumLayers, 0);

    // Allocate/use the layers
    for (int i = 0; i < kInitialNumLayers; ++i) {
        int key[1] = { i + 1 };
        GrCachedLayer* layer = TestingAccess::Find(&cache, picture->uniqueID(), SkMatrix::I(),
                                                   key, 1);
        REPORTER_ASSERT(reporter, layer);

        lock_layer(reporter, &cache, layer);
    }

#if GR_CACHE_STATS
    resourceCache->getStats(&stats);
    REPORTER_ASSERT(reporter, 2 == stats.fTotal);
    REPORTER_ASSERT(reporter, 2 == stats.fNumNonPurgeable);
#endif

    // Unlock the textures
    for (int i = 0; i < kInitialNumLayers; ++i) {
        int key[1] = { i+1 };

        GrCachedLayer* layer = TestingAccess::Find(&cache, picture->uniqueID(), SkMatrix::I(),
                                                   key, 1);
        REPORTER_ASSERT(reporter, layer);
        cache.removeUse(layer);
    }

    picture.reset(nullptr);
    cache.processDeletedPictures();

    REPORTER_ASSERT(reporter, TestingAccess::NumLayers(&cache) == 0);

#if GR_CACHE_STATS
    resourceCache->getStats(&stats);
    REPORTER_ASSERT(reporter, 2 == stats.fTotal);
    REPORTER_ASSERT(reporter, 1 == stats.fNumPurgeable);
    REPORTER_ASSERT(reporter, 1 == stats.fNumNonPurgeable);
#endif

    cache.end();

#if GR_CACHE_STATS
    resourceCache->getStats(&stats);
    REPORTER_ASSERT(reporter, 2 == stats.fTotal);
    REPORTER_ASSERT(reporter, 2 == stats.fNumPurgeable);
#endif
}

#endif
#endif
