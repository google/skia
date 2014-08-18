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
    static int NumLayers(GrLayerCache* cache) {
        return cache->numLayers();
    }
    static void Purge(GrLayerCache* cache, uint32_t pictureID) {
        cache->purge(pictureID);
    }
};

// Add several layers to the cache
static void create_layers(skiatest::Reporter* reporter,
                          GrLayerCache* cache,
                          const SkPicture& picture,
                          int numToAdd,
                          int idOffset) {

    for (int i = 0; i < numToAdd; ++i) {
        GrCachedLayer* layer = cache->findLayerOrCreate(picture.uniqueID(), 
                                                        idOffset+i+1, idOffset+i+2, 
                                                        SkMatrix::I());
        REPORTER_ASSERT(reporter, NULL != layer);
        GrCachedLayer* temp = cache->findLayer(picture.uniqueID(), idOffset+i+1, idOffset+i+2, SkMatrix::I());
        REPORTER_ASSERT(reporter, temp == layer);

        REPORTER_ASSERT(reporter, TestingAccess::NumLayers(cache) == idOffset + i + 1);

        REPORTER_ASSERT(reporter, picture.uniqueID() == layer->pictureID());
        REPORTER_ASSERT(reporter, layer->start() == idOffset + i + 1);
        REPORTER_ASSERT(reporter, layer->stop() == idOffset + i + 2);
        REPORTER_ASSERT(reporter, layer->ctm() == SkMatrix::I());
        REPORTER_ASSERT(reporter, NULL == layer->texture());
        REPORTER_ASSERT(reporter, !layer->isAtlased());
    }

    cache->trackPicture(&picture);
}

static void lock_layer(skiatest::Reporter* reporter,
                       GrLayerCache* cache,
                       GrCachedLayer* layer) {
    // Make the layer 512x512 (so it can be atlased)
    GrTextureDesc desc;
    desc.fWidth = 512;
    desc.fHeight = 512;
    desc.fConfig = kSkia8888_GrPixelConfig;

    bool needsRerendering = cache->lock(layer, desc, false);
    REPORTER_ASSERT(reporter, needsRerendering);

    needsRerendering = cache->lock(layer, desc, false);
    REPORTER_ASSERT(reporter, !needsRerendering);

    REPORTER_ASSERT(reporter, NULL != layer->texture());
    REPORTER_ASSERT(reporter, layer->locked());
}

// This test case exercises the public API of the GrLayerCache class.
// In particular it checks its interaction with the resource cache (w.r.t.
// locking & unlocking textures).
// TODO: need to add checks on VRAM usage!
DEF_GPUTEST(GpuLayerCache, reporter, factory) {
    static const int kInitialNumLayers = 5;

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
        recorder.beginRecording(1, 1);
        SkAutoTUnref<const SkPicture> picture(recorder.endRecording());

        GrLayerCache cache(context);

        create_layers(reporter, &cache, *picture, kInitialNumLayers, 0);

        for (int i = 0; i < kInitialNumLayers; ++i) {
            GrCachedLayer* layer = cache.findLayer(picture->uniqueID(), i+1, i+2, SkMatrix::I());
            REPORTER_ASSERT(reporter, NULL != layer);

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
        for (int i = 0; i < kInitialNumLayers; ++i) {
            GrCachedLayer* layer = cache.findLayer(picture->uniqueID(), i+1, i+2, SkMatrix::I());
            REPORTER_ASSERT(reporter, NULL != layer);

            cache.unlock(layer);
        }

        for (int i = 0; i < kInitialNumLayers; ++i) {
            GrCachedLayer* layer = cache.findLayer(picture->uniqueID(), i+1, i+2, SkMatrix::I());
            REPORTER_ASSERT(reporter, NULL != layer);

            REPORTER_ASSERT(reporter, !layer->locked());
            // The first 4 layers should still be in the atlas.
            if (i < 4) {
                REPORTER_ASSERT(reporter, NULL != layer->texture());
                REPORTER_ASSERT(reporter, layer->isAtlased());
            } else {
                // The final layer should be unlocked.
                REPORTER_ASSERT(reporter, NULL == layer->texture());
                REPORTER_ASSERT(reporter, !layer->isAtlased());
            }
        }

        {
            // Add an additional layer. Since all the layers are unlocked this 
            // will force out the first atlased layer
            create_layers(reporter, &cache, *picture, 1, kInitialNumLayers);
            GrCachedLayer* layer = cache.findLayer(picture->uniqueID(), 
                                                   kInitialNumLayers+1, kInitialNumLayers+2, 
                                                   SkMatrix::I());
            REPORTER_ASSERT(reporter, NULL != layer);

            lock_layer(reporter, &cache, layer);
            cache.unlock(layer);
        }

        for (int i = 0; i < kInitialNumLayers+1; ++i) {
            GrCachedLayer* layer = cache.findLayer(picture->uniqueID(), i+1, i+2, SkMatrix::I());
            // 3 old layers plus the new one should be in the atlas.
            if (1 == i || 2 == i || 3 == i || 5 == i) {
                REPORTER_ASSERT(reporter, NULL != layer);
                REPORTER_ASSERT(reporter, !layer->locked());
                REPORTER_ASSERT(reporter, NULL != layer->texture());
                REPORTER_ASSERT(reporter, layer->isAtlased());
            } else if (4 == i) {
                // The one that was never atlased should still be around
                REPORTER_ASSERT(reporter, NULL != layer);

                REPORTER_ASSERT(reporter, NULL == layer->texture());
                REPORTER_ASSERT(reporter, !layer->isAtlased());
            } else {
                // The one bumped out of the atlas (i.e., 0) should be gone
                REPORTER_ASSERT(reporter, NULL == layer);
            }
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
