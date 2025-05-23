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
#include "src/gpu/graphite/ClipStack.h"
#include "src/gpu/graphite/DrawAtlas.h"
#include "src/gpu/graphite/geom/Rect.h"

namespace skgpu::graphite {

class Recorder;

//////////////////////////////////////////////////////////////////////////////////////////////////
/** The ClipAtlasManager manages the lifetime of and access to rasterized clip masks.
 */
class ClipAtlasManager {
public:
    ClipAtlasManager(Recorder* recorder);
    ~ClipAtlasManager() = default;

    sk_sp<TextureProxy> findOrCreateEntry(uint32_t stackRecordID,
                                          const ClipStack::ElementList*,
                                          SkIRect maskDeviceBounds,
                                          SkIPoint* outPos);

    bool recordUploads(DrawContext* dc);
    void compact();
    void freeGpuResources();

    void evictAtlases();

private:
    // Wrapper class to manage DrawAtlas and associated caching operations
    class DrawAtlasMgr : public AtlasGenerationCounter, public PlotEvictionCallback {
    public:
        DrawAtlasMgr(size_t width, size_t height,
                     size_t plotWidth, size_t plotHeight,
                     DrawAtlas::UseStorageTextures useStorageTextures,
                     std::string_view label, const Caps*);

        sk_sp<TextureProxy> findOrCreateEntry(Recorder* recorder,
                                              const skgpu::UniqueKey&,
                                              const ClipStack::ElementList*,
                                              SkIRect maskDeviceBounds,
                                              SkIRect keyBounds,
                                              SkIPoint* outPos);
        // Adds to DrawAtlas but not the cache
        sk_sp<TextureProxy> addToAtlas(Recorder* recorder,
                                       const ClipStack::ElementList*,
                                       SkIRect maskDeviceBounds,
                                       SkIPoint* outPos,
                                       AtlasLocator* locator);
        bool recordUploads(DrawContext*, Recorder*);
        void evict(PlotLocator) override;
        void compact(Recorder*);
        void freeGpuResources(Recorder*);

        void evictAll();

    private:
        std::unique_ptr<DrawAtlas> fDrawAtlas;

        // Tracks whether a combined clip mask is already in the DrawAtlas and its location
        struct MaskHashEntry {
            SkIRect fBounds;
            AtlasLocator fLocator;
            MaskHashEntry* fNext = nullptr;
        };
        struct UniqueKeyHash {
            uint32_t operator()(const skgpu::UniqueKey& key) const { return key.hash(); }
        };
        using MaskCache = skia_private::THashMap<skgpu::UniqueKey, MaskHashEntry, UniqueKeyHash>;
        MaskCache fMaskCache;
        int fHashEntryCount = 0;

        // List of stored keys per Plot, used to invalidate cache entries.
        // When a Plot is invalidated via evict(), we'll get its index and Page index from the
        // PlotLocator, index into the fKeyLists array to get the MaskKeyList for that Plot,
        // then iterate through the list and remove entries matching those keys from the MaskCache.
        struct MaskKeyEntry {
            skgpu::UniqueKey fKey;
            SkIRect fBounds;
            SK_DECLARE_INTERNAL_LLIST_INTERFACE(MaskKeyEntry);
        };
        using MaskKeyList = SkTInternalLList<MaskKeyEntry>;
        SkTDArray<MaskKeyList> fKeyLists;
        int fListEntryCount = 0;
    };

    Recorder* fRecorder;
    // We have two atlas managers, one for clips that can be keyed via the path keys,
    // and a smaller one for those that can only be keyed by the SaveRecord ID. We keep
    // them separate because the SaveRecord keyed clips will be far more transient, i.e.,
    // once the SaveRecord is popped they'll never be used again.
    DrawAtlasMgr fPathKeyAtlasMgr;
    DrawAtlasMgr fSaveRecordKeyAtlasMgr;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_ClipAtlasManager_DEFINED
