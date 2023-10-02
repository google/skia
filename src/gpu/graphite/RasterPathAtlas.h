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
 * TODO: We should cache Shapes for future frames to avoid the cost of software rendering.
 */
class RasterPathAtlas : public PathAtlas {
public:
    RasterPathAtlas();
    ~RasterPathAtlas() override {}
    void recordUploads(DrawContext*, Recorder*);

protected:
    void onAddShape(const Shape&,
                    const Transform& transform,
                    const Rect& atlasBounds,
                    skvx::int2 deviceOffset,
                    const SkStrokeRec&) override;
    void onReset() override;
    MaskFormat coverageMaskFormat(const Caps*) const override;

    SkAutoPixmapStorage fPixels;
    SkIRect fDirtyRect;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_RasterPathAtlas_DEFINED
