/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_RasterPathAtlas_DEFINED
#define skgpu_graphite_RasterPathAtlas_DEFINED

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
    class RasterAtlasMgr : public PathAtlas::DrawAtlasMgr {
    public:
        RasterAtlasMgr(size_t width, size_t height,
                       size_t plotWidth, size_t plotHeight,
                       const Caps* caps)
            : PathAtlas::DrawAtlasMgr(width, height, plotWidth, plotHeight,
                                      DrawAtlas::UseStorageTextures::kNo,
                                      /*label=*/"RasterPathAtlas", caps) {}

    protected:
        bool onAddToAtlas(const Shape&,
                          const Transform& transform,
                          const SkStrokeRec&,
                          SkIRect shapeBounds,
                          const AtlasLocator&) override;
    };

    RasterAtlasMgr fCachedAtlasMgr;
    RasterAtlasMgr fSmallPathAtlasMgr;
    RasterAtlasMgr fUncachedAtlasMgr;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_RasterPathAtlas_DEFINED
