/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_PathAtlas_DEFINED
#define skgpu_graphite_PathAtlas_DEFINED

#include "include/core/SkStrokeRec.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/gpu/RectanizerSkyline.h"
#include "src/gpu/graphite/geom/CoverageMaskShape.h"

#ifdef SK_ENABLE_VELLO_SHADERS
#include "src/gpu/graphite/compute/VelloRenderer.h"
#endif

#include <memory>

namespace skgpu::graphite {

class Caps;
class DrawContext;
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
     * the rounded out `transformedShapeBounds` offset. For an unclipped shape this amounts to
     * translating it back to its origin while preserving any sub-pixel translation. For a clipped
     * shape, this ensures that the visible portions of the mask are centered in the atlas slot
     * while invisible portions that would lie outside the atlas slot get clipped out.
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
                  const Rect& transformedShapeBounds,
                  const Shape& shape,
                  const Transform& localToDevice,
                  const SkStrokeRec& style,
                  CoverageMaskShape::MaskInfo* outMaskInfo);

    // Clear all scheduled atlas draws and free up atlas allocations. After this call the atlas can
    // be considered cleared and available for new shape insertions. However this method does not
    // have any bearing on the contents of the atlas texture itself, which may be in use by GPU
    // commands that are in-flight or yet to be submitted.
    void reset();

    // Returns a pointer to the atlas texture. Initializes a texture proxy if necessary. Returns
    // nullptr if a texture can not be created.
    const TextureProxy* getTexture(Recorder*);

    uint32_t width() const { return static_cast<uint32_t>(fRectanizer.width()); }
    uint32_t height() const { return static_cast<uint32_t>(fRectanizer.height()); }

protected:
    const TextureProxy* texture() const { return fTexture.get(); }

    virtual void onAddShape(const Shape&,
                            const Transform& transform,
                            const Rect& atlasBounds,
                            skvx::int2 deviceOffset,
                            const SkStrokeRec&) = 0;
    virtual void onReset() = 0;

    struct MaskFormat {
        SkColorType fColorType = kUnknown_SkColorType;
        bool requiresStorageUsage = false;
    };
    virtual MaskFormat coverageMaskFormat(const Caps*) const = 0;

private:
    bool initializeTextureIfNeeded(Recorder*);

    skgpu::RectanizerSkyline fRectanizer;

    // A PathAtlas lazily requests a texture from the AtlasProvider when the first shape gets added
    // to it and references the same texture for the duration of its lifetime. A reference to this
    // texture is stored here, which is used by AtlasShapeRenderStep when encoding the render pass.
    //
    // TODO: Rather than permanently assigning a texture we may want PathAtlases to request one from
    // a pool on demand while encoding a dispatch. Currently all PathAtlases reference the same
    // TextureProxy and the RenderStep can reference it easily via the PathAtlas. We may want to
    // revise how a RenderStep obtains the correct texture if we move to a pooled approach.
    sk_sp<TextureProxy> fTexture;
};

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
    ComputePathAtlas();
    virtual std::unique_ptr<DispatchGroup> recordDispatches(Recorder*) const = 0;

protected:
    MaskFormat coverageMaskFormat(const Caps*) const override;
};

#ifdef SK_ENABLE_VELLO_SHADERS

/**
 * ComputePathAtlas that uses a VelloRenderer.
 */
class VelloComputePathAtlas final : public ComputePathAtlas {
public:
    // Record the compute dispatches that will draw the atlas contents.
    std::unique_ptr<DispatchGroup> recordDispatches(Recorder*) const override;

private:
    void onAddShape(
            const Shape&, const Transform&, const Rect&, skvx::int2, const SkStrokeRec&) override;
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
class SoftwarePathAtlas : public PathAtlas {
public:
    SoftwarePathAtlas();
    ~SoftwarePathAtlas() override {}
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

#endif  // skgpu_graphite_PathAtlas_DEFINED
