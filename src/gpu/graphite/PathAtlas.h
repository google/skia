/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_PathAtlas_DEFINED
#define skgpu_graphite_PathAtlas_DEFINED

#include "include/core/SkStrokeRec.h"
#include "src/base/SkTInternalLList.h"
#include "src/core/SkTHash.h"
#include "src/gpu/AtlasTypes.h"
#include "src/gpu/ResourceKey.h"
#include "src/gpu/graphite/DrawAtlas.h"
#include "src/gpu/graphite/geom/CoverageMaskShape.h"

namespace skgpu::graphite {

class Caps;
class DrawContext;
class Recorder;
class Rect;
class Renderer;
class Shape;
class TextureProxy;
class Transform;

/**
 * PathAtlas manages one or more atlas textures that store coverage masks for path rendering.
 *
 * The contents of a PathAtlas are intended to be transient: atlas regions are considered valid only
 * for the scope of the render passes that sample them. Unlike DrawAtlas, PathAtlas does not
 * necessarily support partial eviction and reuse of subregions. In most subclasses, once an atlas
 * texture is filled up all of its sub-allocations must be invalidated before it can be reused.
 *
 * PathAtlas does not prescribe how atlas contents get uploaded to the GPU. The specific task
 * mechanism is defined by subclasses.
 */
class PathAtlas {
public:
    /**
     * The PathAtlas will use textures of the requested size or the system's maximum texture size,
     * whichever is smaller.
     */
    PathAtlas(Recorder* recorder, uint32_t requestedWidth, uint32_t requestedHeight);
    virtual ~PathAtlas();

    using MaskAndOrigin = std::pair<CoverageMaskShape, SkIPoint>;

    // Subclasses should ensure that the recorded masks have this much padding around each entry.
    // PathAtlas passes in un-padded sizes to onAddShape and assumes that padding has been included
    // in the outPos value.
    static constexpr int kEntryPadding = 1;

    /**
     * Searches the atlas for a slot that can fit a coverage mask for a clipped shape with the given
     * bounds in device coordinates and submits the mask to be drawn into the found atlas region.
     * For atlases that cache coverage masks, will first search the cache before adding.
     *
     * Returns an empty result if a the shape cannot fit in the atlas. Otherwise, returns the
     * CoverageMaskShape (including the texture proxy) for sampling the eventually-rendered coverage
     * mask and the device-space origin the mask should be drawn at (e.g. its recorded draw should
     * be an integer translation matrix), and the Renderer that should be used to draw that shape.
     * The Renderer should have single-channel coverage, require AA bounds outsetting, and have a
     * single renderStep.
     *
     * The bounds of the atlas entry is laid out with a 1 pixel outset from the given dimensions.
     * The returned shape's UV origin accounts for the padding, and its mask size does not include
     * the padding. This allows the mask to be sampled safely with linear filtering without worrying
     * about HW filtering accessing pixels from other entries.
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
     */
    std::pair<const Renderer*, std::optional<MaskAndOrigin>> addShape(
            const Rect& transformedShapeBounds,
            const Shape& shape,
            const Transform& localToDevice,
            const SkStrokeRec& style);

    /**
     * Returns true if a path coverage mask with the given device-space bounds is sufficiently
     * small to benefit from atlasing without causing too many atlas renders.
     *
     * `transformedShapeBounds` represents the device-space bounds of the coverage mask shape
     * unrestricted by clip and viewport bounds.
     *
     * `clipBounds` represents the conservative bounding box of the union of the clip stack that
     * should apply to the shape.
     */
    virtual bool isSuitableForAtlasing(const Rect& transformedShapeBounds,
                                       const Rect& clipBounds) const {
        return true;
    }

    uint32_t width() const { return fWidth; }
    uint32_t height() const { return fHeight; }

protected:
    // The 'transform' has been adjusted to draw the Shape into a logical image from (0,0) to
    // 'maskSize'. The actual rendering into the returned TextureProxy will need to be further
    // translated by the value written to 'outPos', which is the responsibility of subclasses.
    virtual const TextureProxy* onAddShape(const Shape&,
                                           const Transform& transform,
                                           const SkStrokeRec&,
                                           skvx::half2 maskSize,
                                           skvx::half2* outPos) = 0;

    // Wrapper class to manage DrawAtlas and associated caching operations
    class DrawAtlasMgr : public AtlasGenerationCounter, public PlotEvictionCallback {
    public:
        const TextureProxy* findOrCreateEntry(Recorder* recorder,
                                              const Shape& shape,
                                              const Transform& transform,
                                              const SkStrokeRec& strokeRec,
                                              skvx::half2 maskSize,
                                              skvx::half2* outPos);
        // Adds to DrawAtlas but not the cache
        const TextureProxy* addToAtlas(Recorder* recorder,
                                       const Shape& shape,
                                       const Transform& transform,
                                       const SkStrokeRec& strokeRec,
                                       skvx::half2 maskSize,
                                       skvx::half2* outPos,
                                       AtlasLocator* locator);
        bool recordUploads(DrawContext*, Recorder*);
        void evict(PlotLocator) override;
        void postFlush(Recorder*);

    protected:
        DrawAtlasMgr(size_t width, size_t height,
                     size_t plotWidth, size_t plotHeight,
                     DrawAtlas::UseStorageTextures useStorageTextures,
                     std::string_view label, const Caps*);

        bool virtual onAddToAtlas(const Shape&,
                                  const Transform& transform,
                                  const SkStrokeRec&,
                                  SkIRect shapeBounds,
                                  const AtlasLocator&) = 0;

        std::unique_ptr<DrawAtlas> fDrawAtlas;

    private:
        // Tracks whether a shape is already in the DrawAtlas, and its location in the atlas
        struct UniqueKeyHash {
            uint32_t operator()(const skgpu::UniqueKey& key) const { return key.hash(); }
        };
        using ShapeCache = skia_private::THashMap<skgpu::UniqueKey, AtlasLocator, UniqueKeyHash>;
        ShapeCache fShapeCache;

        // List of stored keys per Plot, used to invalidate cache entries.
        // When a Plot is invalidated via evict(), we'll get its index and Page index from the
        // PlotLocator, index into the fKeyLists array to get the ShapeKeyList for that Plot,
        // then iterate through the list and remove entries matching those keys from the ShapeCache.
        struct ShapeKeyEntry {
            skgpu::UniqueKey fKey;
            SK_DECLARE_INTERNAL_LLIST_INTERFACE(ShapeKeyEntry);
        };
        using ShapeKeyList = SkTInternalLList<ShapeKeyEntry>;
        SkTDArray<ShapeKeyList> fKeyLists;
    };

    // The Recorder that created and owns this Atlas.
    Recorder* fRecorder;

    uint32_t fWidth = 0;
    uint32_t fHeight = 0;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_PathAtlas_DEFINED
