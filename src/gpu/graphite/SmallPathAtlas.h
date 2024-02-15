/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_SmallPathAtlas_DEFINED
#define skgpu_graphite_SmallPathAtlas_DEFINED

#include "src/base/SkTInternalLList.h"
#include "src/core/SkTHash.h"
#include "src/gpu/ResourceKey.h"
#include "src/gpu/graphite/DrawAtlas.h"
#include "src/gpu/graphite/PathAtlas.h"

namespace skgpu::graphite {

class DrawContext;

/**
 * PathAtlas class that rasterizes coverage masks on the CPU and caches them in a
 * DrawAtlas. Only masks below a certain size will be stored in this atlas; larger
 * masks should be stored in the RasterPathAtlas.
 *
 * When a new shape gets added, its path is rasterized in preparation for upload. These
 * uploads are recorded by `recordUploads()` and subsequently added to an UploadTask.
 *
 */
class SmallPathAtlas : public PathAtlas,
                       public AtlasGenerationCounter,
                       public PlotEvictionCallback {
public:
    explicit SmallPathAtlas(Recorder*);
    ~SmallPathAtlas() override {}

    bool initAtlas();

    bool recordUploads(DrawContext*);

protected:
    const TextureProxy* onAddShape(const Shape&,
                                   const Transform& transform,
                                   const SkStrokeRec&,
                                   skvx::half2 maskSize,
                                   skvx::half2* outPos) override;

private:
    void evict(PlotLocator) override;

    // TODO: select atlas size dynamically? Take ContextOptions::fMaxTextureAtlasSize into account?
    static constexpr int kDefaultAtlasDim = 2048;

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
    // then iterate through that list and remove entries matching those keys from the ShapeCache.
    struct ShapeKeyEntry {
        skgpu::UniqueKey fKey;
        SK_DECLARE_INTERNAL_LLIST_INTERFACE(ShapeKeyEntry);
    };
    using ShapeKeyList = SkTInternalLList<ShapeKeyEntry>;
    SkTDArray<ShapeKeyList> fKeyLists;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_SmallPathAtlas_DEFINED
