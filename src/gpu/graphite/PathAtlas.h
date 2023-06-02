/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_PathAtlas_DEFINED
#define skgpu_graphite_PathAtlas_DEFINED

#include "include/core/SkStrokeRec.h"
#include "src/gpu/RectanizerSkyline.h"

#ifdef SK_ENABLE_VELLO_SHADERS
#include "src/gpu/graphite/compute/VelloRenderer.h"
#endif

#include <memory>

namespace skgpu::graphite {

class Recorder;
class Rect;
class Shape;
class TextureProxy;
class Transform;

/**
 * PathAtlas manages one or more atlas textures that store coverage masks for path rendering.
 *
 * The contents of a PathAtlas are intended to be transient: atlas regions are considered valid only
 * for the scope of the render passes that sample them. Unlike DrawAtlas, PathAtlas does not support
 * partial eviction and reuse of subregions. Once an atlas texture is filled up, all of its
 * sub-allocations must be invalidated before it can be reused.
 *
 * PathAtlas does not prescribe how atlas contents get uploaded to the GPU. The specific task
 * mechanism is defined by subclasses.
 */
class PathAtlas {
public:
    PathAtlas(uint32_t width, uint32_t height);
    virtual ~PathAtlas();

    /**
     * Searches the atlas for a slot that can fit a coverage mask for a clipped shape with the given
     * bounds in device coordinates and submits the mask to be drawn into the found atlas region.
     *
     * Returns false if a the shape cannot fit in the atlas. Otherwise, returns true and populates
     * `outAtlasBounds` with the atlas-space boundaries of the mask region.
     *
     * The bounds of the atlas entry is laid out with a 1 pixel outset from the given dimensions.
     *
     * `shape` will be drawn after applying the linear components (scale, rotation, skew) of the
     * provided `localToDevice` transform. This is done by  translating the shape by the inverse of
     * the `maskBounds` offset. For an unclipped shape this amounts to translating it back to its
     * origin. For a clipped shape, this ensures that the visible portions of the mask are centered
     * in the atlas slot while invisible portions that would lie outside the atlas slot are clipped
     * out.
     *
     * `addShape()` schedules the shape to be drawn but when and how the rendering happens is
     * specified by the subclass implementation.
     *
     * The stroke-and-fill style is drawn as a single combined coverage mask containing the stroke
     * and the fill.
     *
     * This method lazily creates a TextureProxy that can be referenced by tasks that want to sample
     * the atlas.
     */
    bool addShape(Recorder*,
                  const Rect& maskBounds,
                  const Shape& shape,
                  const Transform& localToDevice,
                  const SkStrokeRec& style,
                  Rect* outAtlasBounds);

    // Clear all scheduled atlas draws and free up atlas allocations. After this call the atlas can
    // be considered cleared and available for new shape insertions. However this method does not
    // have any bearing on the contents of the atlas texture itself, which may be in use by GPU
    // commands that are in-flight or yet to be submitted.
    void reset();

    // Returns a pointer to the atlas texture.
    const TextureProxy* texture() const { return fTexture.get(); }

    uint32_t width() const { return static_cast<uint32_t>(fRectanizer.width()); }
    uint32_t height() const { return static_cast<uint32_t>(fRectanizer.height()); }

protected:
    virtual void onAddShape(const Shape&,
                            const Transform& transform,
                            const Rect& atlasBounds,
                            float deviceOffsetX,
                            float deviceOffsetY,
                            const SkStrokeRec&) = 0;
    virtual void onReset() = 0;

private:
    skgpu::RectanizerSkyline fRectanizer;

    // PathAtlas currently supports a single atlas page. The TextureProxy gets created only once and
    // is valid for the lifetime of the PathAtlas. It is possible for the same texture to be bound
    // to multiple DispatchGroups and DrawPasses across flushes. The caller must make sure that any
    // uploads or compute dispatches are scheduled to remain coherent across flushes.
    sk_sp<TextureProxy> fTexture;
};

// NOTE: currently the coverage mask atlas that uses GPU compute uses Vello shaders, so its
// availability is conditioned on SK_ENABLE_VELLO_SHADERS:
#ifdef SK_ENABLE_VELLO_SHADERS

class DispatchGroup;

/**
 * PathAtlas implementation that rasterizes the coverage masks on the GPU using compute shaders.
 *
 * When a new shape gets added, it gets encoded into data streams that will later serve as an input
 * to a series of GPU compute passes. This data is recorded into a DispatchGroup which can be added
 * to a ComputeTask in `recordDispatches()`.
 *
 * After a successful call to `recordDispatches()`, the client is free to call `reset()` and start
 * adding new shapes for a future atlas render.
 */
class ComputePathAtlas final : public PathAtlas {
public:
    ComputePathAtlas();

    // Record the compute dispatches that will draw the atlas contents.
    std::unique_ptr<DispatchGroup> recordDispatches(Recorder*) const;

private:
    void onAddShape(const Shape&, const Transform&, const Rect&,
                    float, float, const SkStrokeRec&) override;
    void onReset() override {
        fScene.reset();
        fOccuppiedWidth = fOccuppiedHeight = 0;
    }

    // Contains the encoded scene buffer data that serves as the input to a vello compute pass.
    VelloScene fScene;

    // Occuppied bounds of the atlas
    uint32_t fOccuppiedWidth = 0;
    uint32_t fOccuppiedHeight = 0;
};

#endif  // SK_ENABLE_VELLO_SHADERS

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_PathAtlas_DEFINED
