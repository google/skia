/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_RasterPathAtlas_DEFINED
#define skgpu_graphite_RasterPathAtlas_DEFINED

#include "src/base/SkTInternalLList.h"
#include "src/core/SkTHash.h"
#include "src/gpu/AtlasTypes.h"
#include "src/gpu/ResourceKey.h"
#include "src/gpu/graphite/DrawAtlas.h"
#include "src/gpu/graphite/PathAtlas.h"

namespace skgpu::graphite {

/**
 * PathAtlas class that rasterizes coverage masks on the CPU.
 *
 * When a new shape gets added, its path is rasterized in preparation for upload. These
 * uploads are recorded by `recordUploads()` and subsequently added to an UploadTask.
 *
 * Shapes are cached for future frames to avoid the cost of raster pipeline rendering. Multiple
 * textures (or Pages) are used to cache masks, so if the atlas is full we can reset a Page and
 * start adding new shapes for a future atlas render.
 */
class RasterPathAtlas : public PathAtlas {
public:
    explicit RasterPathAtlas(Recorder* recorder);
    ~RasterPathAtlas() override {}
    void recordUploads(DrawContext*);

    void postFlush() {
        fCachedAtlasMgr.postFlush(fRecorder);
        fSmallPathAtlasMgr.postFlush(fRecorder);
        fUncachedAtlasMgr.postFlush(fRecorder);
    }

protected:
    const TextureProxy* onAddShape(const Shape&,
                                   const Transform& transform,
                                   const SkStrokeRec&,
                                   skvx::half2 maskSize,
                                   skvx::half2* outPos) override;
private:
    // Wrapper class to manage DrawAtlas and associated caching operations
    class DrawAtlasMgr : public AtlasGenerationCounter, public PlotEvictionCallback {
    public:
        DrawAtlasMgr(size_t width, size_t height,
                     size_t plotWidth, size_t plotHeight,
                     const Caps*);

        const TextureProxy* findOrCreateEntry(Recorder* recorder,
                                              const Shape& shape,
                                              const Transform& transform,
                                              const SkStrokeRec& strokeRec,
                                              skvx::half2 maskSize,
                                              skvx::half2* outPos);
        // Adds to DrawAtlas but not the cache
        const TextureProxy* addToAtlas(Recorder* recorder,
                                       const Shape& shape,
                                       const Transform& transform,
                                       const SkStrokeRec& strokeRec,
                                       skvx::half2 maskSize,
                                       skvx::half2* outPos,
                                       AtlasLocator* locator);
        bool recordUploads(DrawContext*, Recorder*);
        void evict(PlotLocator) override;
        void postFlush(Recorder*);

    private:

        std::unique_ptr<DrawAtlas> fDrawAtlas;

        // Tracks whether a shape is already in the DrawAtlas, and its location in the atlas
        struct UniqueKeyHash {
            uint32_t operator()(const skgpu::UniqueKey& key) const { return key.hash(); }
        };
        using ShapeCache = skia_private::THashMap<skgpu::UniqueKey, AtlasLocator, UniqueKeyHash>;
        ShapeCache fShapeCache;

        // List of stored keys per Plot, used to invalidate cache entries.
        // When a Plot is invalidated via evict(), we'll get its index and Page index from the
        // PlotLocator, index into the fKeyLists array to get the ShapeKeyList for that Plot,
        // then iterate through the list and remove entries matching those keys from the ShapeCache.
        struct ShapeKeyEntry {
            skgpu::UniqueKey fKey;
            SK_DECLARE_INTERNAL_LLIST_INTERFACE(ShapeKeyEntry);
        };
        using ShapeKeyList = SkTInternalLList<ShapeKeyEntry>;
        SkTDArray<ShapeKeyList> fKeyLists;
    };

    DrawAtlasMgr fCachedAtlasMgr;
    DrawAtlasMgr fSmallPathAtlasMgr;
    DrawAtlasMgr fUncachedAtlasMgr;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_RasterPathAtlas_DEFINED
