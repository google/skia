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
#include "src/gpu/ResourceKey.h"
#include "src/gpu/graphite/PathAtlas.h"

namespace skgpu::graphite {

/**
 * PathAtlas class that rasterizes coverage masks on the CPU.
 *
 * When a new shape gets added, its path is rasterized in preparation for upload. These
 * uploads are recorded by `recordUploads()` and subsequently added to an UploadTask.
 *
 * After a successful call to `recordUploads()`, the client is free to call `reset()` and start
 * adding new shapes for a future atlas render.
 * TODO: We should cache Shapes for future frames to avoid the cost of raster pipeline rendering.
 */
class RasterPathAtlas : public PathAtlas {
public:
    RasterPathAtlas();
    ~RasterPathAtlas() override {}
    void recordUploads(DrawContext*, Recorder*);

    // Clear all scheduled atlas draws and free up atlas allocations, if necessary. After this call
    // the atlas can be considered cleared and available for new shape insertions. However this
    // method does not have any bearing on the contents of any atlas textures themselves, which may
    // be in use by GPU commands that are in-flight or yet to be submitted.
    // TODO: can probably remove this once caching is working, or only use it for the LRU page
    // when out of space
    void reset();

protected:
    const TextureProxy* onAddShape(Recorder* recorder,
                                   const Shape&,
                                   const Transform& transform,
                                   const SkStrokeRec&,
                                   skvx::float2 atlasSize,
                                   skvx::int2 deviceOffset,
                                   skvx::half2* outPos) override;
    const TextureProxy* addRect(Recorder* recorder,
                                skvx::float2 atlasSize,
                                SkIPoint16* outPos);

private:
    // TODO: select atlas size dynamically? Take ContextOptions::fMaxTextureAtlasSize into account?
    static constexpr int kDefaultAtlasDim = 4096;

    struct Page {
        bool initializeTextureIfNeeded(Recorder* recorder);

        // A Page lazily requests a texture from the AtlasProvider when the first shape gets added
        // to it and references the same texture for the duration of its lifetime. A reference to
        // this texture is stored here, which is used by CoverageMaskRenderStep when encoding the
        // render pass.
        sk_sp<TextureProxy> fTexture;
        // Tracks placement of paths in a Page
        skgpu::RectanizerSkyline fRectanizer = {kDefaultAtlasDim, kDefaultAtlasDim};
        // Rendered data that gets uploaded
        SkAutoPixmapStorage fPixels;
        // Area that's needed to be uploaded
        SkIRect fDirtyRect;
        // Tracks whether a path is already in this Page, and its location in the atlas
        struct UniqueKeyHash {
            uint32_t operator()(const skgpu::UniqueKey& key) const { return key.hash(); }
        };
        skia_private::THashMap<skgpu::UniqueKey, skvx::half2, UniqueKeyHash> fCachedShapes;
        bool fNeedsReset = false;

        SK_DECLARE_INTERNAL_LLIST_INTERFACE(Page);
    };

    static constexpr int kMaxPages = 1;  // TODO: pick value - 4?
    // LRU list of Pages (MRU at head - LRU at tail)
    SkTInternalLList<Page> fPageList;
    // Allocated array of pages (backing data for list)
    Page fPageArray[kMaxPages];
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_RasterPathAtlas_DEFINED
