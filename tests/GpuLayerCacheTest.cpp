/*
* Copyright 2014 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#if SK_SUPPORT_GPU

#include "GrContext.h"
#include "GrContextFactory.h"
#include "GrLayerCache.h"
#include "SkPictureRecorder.h"
#include "Test.h"

class TestingAccess {
public:
    static unsigned NumLayers(GrLayerCache* cache) {
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
                               const unsigned* key, int keySize) {
        return cache->findLayer(pictureID, initialMat, key, keySize);
    }
};

// Add several layers to the cache
static void create_layers(skiatest::Reporter* reporter,
                          GrLayerCache* cache,
                          const SkPicture& picture,
                          unsigned numToAdd,
                          unsigned idOffset) {

    for (unsigned i = 0; i < numToAdd; ++i) {
        unsigned indices[1] = { idOffset+i+1 };
        GrCachedLayer* layer = cache->findLayerOrCreate(picture.uniqueID(), 
                                                        idOffset+i+1, idOffset+i+2, 
                                                        SkIRect::MakeEmpty(),
                                                        SkIRect::MakeEmpty(),
                                                        SkMatrix::I(),
                                                        indices, 1,
                                                        NULL);
        REPORTER_ASSERT(reporter, layer);
        GrCachedLayer* temp = TestingAccess::Find(cache, picture.uniqueID(), SkMatrix::I(), 
                                                  indices, 1);
        REPORTER_ASSERT(reporter, temp == layer);

        REPORTER_ASSERT(reporter, TestingAccess::NumLayers(cache) == idOffset + i + 1);

        REPORTER_ASSERT(reporter, picture.uniqueID() == layer->pictureID());
        REPORTER_ASSERT(reporter, layer->start() == idOffset + i + 1);
        REPORTER_ASSERT(reporter, layer->stop() == idOffset + i + 2);
        REPORTER_ASSERT(reporter, NULL == layer->texture());
        REPORTER_ASSERT(reporter, NULL == layer->paint());
        REPORTER_ASSERT(reporter, !layer->isAtlased());
    }
}

static void lock_layer(skiatest::Reporter* reporter,
                       GrLayerCache* cache,
                       GrCachedLayer* layer) {
    // Make the layer 512x512 (so it can be atlased)
    GrSurfaceDesc desc;
    desc.fWidth = 512;
    desc.fHeight = 512;
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
DEF_GPUTEST(GpuLayerCache, reporter, factory) {
    static const unsigned kInitialNumLayers = 5;

    for (int i= 0; i < GrContextFactory::kGLContextTypeCnt; ++i) {
        GrContextFactory::GLContextType glCtxType = (GrContextFactory::GLContextType) i;

        if (!GrContextFactory::IsRenderingGLContext(glCtxType)) {
            continue;
        }

        GrContext* context = factory->get(glCtxType);

        if (NULL == context) {
            continue;
        }

        SkPictureRecorder recorder;
        SkCanvas* c = recorder.beginRecording(1, 1);
            // Draw something, anything, to prevent an empty-picture optimization,
            // which is a singleton and never purged.
            c->drawRect(SkRect::MakeWH(1,1), SkPaint());
        SkAutoTUnref<const SkPicture> picture(recorder.endRecording());

        GrLayerCache cache(context);

        create_layers(reporter, &cache, *picture, kInitialNumLayers, 0);

        for (unsigned i = 0; i < kInitialNumLayers; ++i) {
            unsigned indices[1] = { i + 1 };
            GrCachedLayer* layer = TestingAccess::Find(&cache, picture->uniqueID(), SkMatrix::I(),
                                                       indices, 1);
            REPORTER_ASSERT(reporter, layer);

            lock_layer(reporter, &cache, layer);

            // The first 4 layers should be in the atlas (and thus have non-empty
            // rects)
            if (i < 4) {
                REPORTER_ASSERT(reporter, layer->isAtlased());
            } else {
                // The 5th layer couldn't fit in the atlas
                REPORTER_ASSERT(reporter, !layer->isAtlased());
            }
        }

        // Unlock the textures
        for (unsigned i = 0; i < kInitialNumLayers; ++i) {
            unsigned indices[1] = { i+1 };

            GrCachedLayer* layer = TestingAccess::Find(&cache, picture->uniqueID(), SkMatrix::I(),
                                                       indices, 1);
            REPORTER_ASSERT(reporter, layer);
            cache.removeUse(layer);
        }

        for (unsigned i = 0; i < kInitialNumLayers; ++i) {
            unsigned indices[1] = { i+1 };

            GrCachedLayer* layer = TestingAccess::Find(&cache, picture->uniqueID(), SkMatrix::I(),
                                                       indices, 1);
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
                REPORTER_ASSERT(reporter, NULL == layer->texture());
                REPORTER_ASSERT(reporter, !layer->isAtlased());
#if GR_CACHE_HOISTED_LAYERS
            }
#endif
        }

        {
            unsigned indices[1] = { kInitialNumLayers+1 };

            // Add an additional layer. Since all the layers are unlocked this 
            // will force out the first atlased layer
            create_layers(reporter, &cache, *picture, 1, kInitialNumLayers);
            GrCachedLayer* layer = TestingAccess::Find(&cache, picture->uniqueID(), SkMatrix::I(),
                                                       indices, 1);
            REPORTER_ASSERT(reporter, layer);

            lock_layer(reporter, &cache, layer);
            cache.removeUse(layer);
        }

        for (unsigned i = 0; i < kInitialNumLayers+1; ++i) {
            unsigned indices[1] = { i+1 };

            GrCachedLayer* layer = TestingAccess::Find(&cache, picture->uniqueID(), SkMatrix::I(),
                                                       indices, 1);
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

                REPORTER_ASSERT(reporter, NULL == layer->texture());
                REPORTER_ASSERT(reporter, !layer->isAtlased());
#if GR_CACHE_HOISTED_LAYERS
            } else {
                // The one bumped out of the atlas (i.e., 0) should be gone
                REPORTER_ASSERT(reporter, NULL == layer);
            }
#endif
        }

        //--------------------------------------------------------------------
        // Free them all SkGpuDevice-style. This will not free up the
        // atlas' texture but will eliminate all the layers.
        TestingAccess::Purge(&cache, picture->uniqueID());

        REPORTER_ASSERT(reporter, TestingAccess::NumLayers(&cache) == 0);
        // TODO: add VRAM/resource cache check here

        //--------------------------------------------------------------------
        // Test out the GrContext-style purge. This should remove all the layers
        // and the atlas.
        // Re-create the layers
        create_layers(reporter, &cache, *picture, kInitialNumLayers, 0);

        // Free them again GrContext-style. This should free up everything.
        cache.freeAll();

        REPORTER_ASSERT(reporter, TestingAccess::NumLayers(&cache) == 0);
        // TODO: add VRAM/resource cache check here

        //--------------------------------------------------------------------
        // Test out the MessageBus-style purge. This will not free the atlas
        // but should eliminate the free-floating layers.
        create_layers(reporter, &cache, *picture, kInitialNumLayers, 0);

        picture.reset(NULL);
        cache.processDeletedPictures();

        REPORTER_ASSERT(reporter, TestingAccess::NumLayers(&cache) == 0);
        // TODO: add VRAM/resource cache check here
    }
}

#endif
