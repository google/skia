/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ComputePathAtlas_DEFINED
#define skgpu_graphite_ComputePathAtlas_DEFINED

#include "src/gpu/graphite/PathAtlas.h"

#include "src/base/SkTInternalLList.h"
#include "src/core/SkTHash.h"
#include "src/gpu/AtlasTypes.h"
#include "src/gpu/RectanizerSkyline.h"
#include "src/gpu/ResourceKey.h"
#include "src/gpu/graphite/DrawAtlas.h"
#include "src/gpu/graphite/task/ComputeTask.h"

#ifdef SK_ENABLE_VELLO_SHADERS
#include "src/gpu/graphite/compute/VelloRenderer.h"
#endif

#include <memory>

namespace skgpu::graphite {

class DispatchGroup;

/**
 * Base class for PathAtlas implementations that rasterize coverage masks on the GPU using compute
 * shaders.
 *
 * When a new shape gets added, it gets tracked as input to a series of GPU compute passes. This
 * data is recorded by `recordDispatches()` into a DispatchGroup which can be added to a
 * ComputeTask.
 *
 * After a successful call to `recordDispatches()`, the client is free to call `reset()` and start
 * adding new shapes for a future atlas render.
 */
class ComputePathAtlas : public PathAtlas {
public:
    // Returns the currently preferred ComputePathAtlas implementation.
    static std::unique_ptr<ComputePathAtlas> CreateDefault(Recorder*);

    virtual bool recordDispatches(Recorder*, ComputeTask::DispatchGroupList*) const = 0;

    // Clear all scheduled atlas draws and free up atlas allocations, if necessary. After this call
    // the atlas can be considered cleared and available for new shape insertions. However this
    // method does not have any bearing on the contents of any atlas textures themselves, which may
    // be in use by GPU commands that are in-flight or yet to be submitted.
    void reset();

protected:
    explicit ComputePathAtlas(Recorder*);

    const TextureProxy* texture() const { return fTexture.get(); }
    sk_sp<TextureProxy> addRect(skvx::half2 maskSize,
                                SkIPoint16* outPos);
    bool isSuitableForAtlasing(const Rect& transformedShapeBounds,
                               const Rect& clipBounds) const override;

    virtual void onReset() = 0;

private:
    bool initializeTextureIfNeeded();

    //////////////////
    // Uncached data
    skgpu::RectanizerSkyline fRectanizer;

    // ComputePathAtlas lazily requests a texture from the AtlasProvider when the first shape gets
    // added to it and references the same texture for the duration of its lifetime. A reference to
    // this texture is stored here, which is used by AtlasShapeRenderStep when encoding the render
    // pass.
    sk_sp<TextureProxy> fTexture;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_ComputePathAtlas_DEFINED
