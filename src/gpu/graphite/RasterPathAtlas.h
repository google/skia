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
    // TODO: can probably remove this once caching is working
    void reset();

protected:
    void onAddShape(const Shape&,
                    const Transform& transform,
                    const Rect& atlasBounds,
                    skvx::int2 deviceOffset,
                    const SkStrokeRec&) override;
    const TextureProxy* addRect(Recorder* recorder,
                                skvx::float2 atlasSize,
                                SkIPoint16* pos) override;

private:
    const TextureProxy* texture() const { return fTexture.get(); }
    bool initializeTextureIfNeeded(Recorder*);

    skgpu::RectanizerSkyline fRectanizer;
    SkAutoPixmapStorage fPixels;
    SkIRect fDirtyRect;

    // RasterPathAtlas lazily requests a texture from the AtlasProvider when the first shape gets
    // added to it and references the same texture for the duration of its lifetime. A reference to
    // this texture is stored here, which is used by AtlasShapeRenderStep when encoding the render
    // pass.
    // TODO: Store list of TextureProxys and Rectanizers that can be used to cache shapes instead.
    sk_sp<TextureProxy> fTexture;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_RasterPathAtlas_DEFINED
