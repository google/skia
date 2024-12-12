/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ClipAtlasManager_DEFINED
#define skgpu_graphite_ClipAtlasManager_DEFINED

#include "include/gpu/graphite/TextureInfo.h"
#include "src/base/SkTInternalLList.h"
#include "src/gpu/AtlasTypes.h"
#include "src/gpu/ResourceKey.h"
#include "src/gpu/graphite/ClipStack_graphite.h"
#include "src/gpu/graphite/DrawAtlas.h"
#include "src/gpu/graphite/geom/Rect.h"

namespace skgpu::graphite {

class Recorder;

//////////////////////////////////////////////////////////////////////////////////////////////////
/** The ClipAtlasManager manages the lifetime of and access to rasterized clip masks.
 */
class ClipAtlasManager : public AtlasGenerationCounter, public PlotEvictionCallback {
public:
    ClipAtlasManager(Recorder* recorder);
    ~ClipAtlasManager() override = default;

    std::tuple<const TextureProxy*, Rect> findClip(const UniqueKey&);
    std::tuple<const TextureProxy*, Rect> addClip(const UniqueKey&, Rect bounds,
                                                  const ClipStack::ElementList*);

    bool recordUploads(DrawContext* dc);
    void evict(PlotLocator) override;
    void compact(bool forceCompact);

    void evictAll();

private:
    Recorder* fRecorder;
    std::unique_ptr<DrawAtlas> fDrawAtlas;

    // Tracks whether a clip mask is already in the DrawAtlas, and its location in the atlas
    struct UniqueKeyHash {
        uint32_t operator()(const skgpu::UniqueKey& key) const { return key.hash(); }
    };
    using MaskCache = skia_private::THashMap<skgpu::UniqueKey, AtlasLocator, UniqueKeyHash>;
    MaskCache fMaskCache;

    // List of stored keys per Plot, used to invalidate cache entries.
    // When a Plot is invalidated via evict(), we'll get its index and Page index from the
    // PlotLocator, index into the fKeyLists array to get the MaskKeyList for that Plot,
    // then iterate through the list and remove entries matching those keys from the MaskCache.
    struct MaskKeyEntry {
        skgpu::UniqueKey fKey;
        SK_DECLARE_INTERNAL_LLIST_INTERFACE(MaskKeyEntry);
    };
    using MaskKeyList = SkTInternalLList<MaskKeyEntry>;
    SkTDArray<MaskKeyList> fKeyLists;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_ClipAtlasManager_DEFINED
