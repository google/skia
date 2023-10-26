/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_geom_CoverageMaskShape_DEFINED
#define skgpu_graphite_geom_CoverageMaskShape_DEFINED

#include "src/base/SkVx.h"
#include "src/gpu/graphite/geom/Rect.h"
#include "src/gpu/graphite/geom/Shape.h"

namespace skgpu::graphite {

class TextureProxy;

/**
 * CoverageMaskShape represents a shape for which per-pixel coverage data comes from a
 * texture. This excludes font glyphs that are rendered to a persistent atlas, as those are
 * represented by the SubRunData geometry type.
 *
 * Coverage masks are defined relative to an intermediate coordinate space between the final
 * device space and the original geometry and shading's local space. For atlases and simple cases
 * this intermediate space is pixel-aligned with the final device space, meaning only an integer
 * translation is necessary to align the mask with where the original geometry would have been
 * rendered into the device. In complex cases, the remaining transform may include rotation, skew,
 * or even perspective that has to be applied after some filter effect.
 *
 * Regardless, the DrawParams that records the CoverageMaskShape stores this remaining transform as
 * the "local-to-device" tranform, i.e. "local" refers to the mask's coordinate space. The
 * CoverageMaskShape stores the original local-to-device inverse so that it can reconstruct coords
 * for shading. Like other Geometry types, the bounds() returned by CoverageMaskShape are relative
 * to its local space, so they are identical to its mask size.
 */
class CoverageMaskShape {
    using half2 = skvx::half2;
    using int2 = skvx::int2;

public:
    struct MaskInfo {
        // The texture-relative integer UV coordinates of the top-left corner of this shape's
        // coverage mask bounds. This will include the rounded out transformed device space bounds
        // of the shape plus a 1-pixel border.
        half2 fTextureOrigin;

        // The width and height of the bounds of the coverage mask shape in device coordinates. This
        // includes the rounded out transformed device space bounds of the shape + a 1-pixel border
        // added for AA.
        half2 fMaskSize;
    };

    CoverageMaskShape() = default;
    CoverageMaskShape(const Shape& shape,
                      const TextureProxy* proxy,
                      const SkM44& deviceToLocal,
                      const MaskInfo& maskInfo)
            : fTextureProxy(proxy)
            , fDeviceToLocal(deviceToLocal)
            , fInverted(shape.inverted())
            , fMaskInfo(maskInfo) {
        SkASSERT(proxy);
    }
    CoverageMaskShape(const CoverageMaskShape&) = default;

    ~CoverageMaskShape() = default;

    // NOTE: None of the geometry types benefit from move semantics, so we don't bother
    // defining a move assignment operator for CoverageMaskShape.
    CoverageMaskShape& operator=(CoverageMaskShape&&) = delete;
    CoverageMaskShape& operator=(const CoverageMaskShape&) = default;

    // Returns the mask-space bounds of the clipped coverage mask shape. For inverse fills this
    // is different from the actual draw bounds stored in the Clip.
    Rect bounds() const {
        return Rect(0.f, 0.f, (float) this->maskSize().x(), (float) this->maskSize().y());
    }

    // The inverse local-to-device matrix.
    const SkM44& deviceToLocal() const { return fDeviceToLocal; }

    // The texture-relative integer UV coordinates of the top-left corner of this shape's
    // coverage mask bounds.
    const half2& textureOrigin() const { return fMaskInfo.fTextureOrigin; }

    // The width and height of the bounds of the coverage mask shape in device coordinates.
    const half2& maskSize() const { return fMaskInfo.fMaskSize; }

    // The texture that the shape will be rendered to.
    const TextureProxy* textureProxy() const { return fTextureProxy; }

    // Whether or not the shape will be painted according to an inverse fill rule.
    bool inverted() const { return fInverted; }

private:
    const TextureProxy* fTextureProxy;
    SkM44 fDeviceToLocal;
    bool fInverted;
    MaskInfo fMaskInfo;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_geom_CoverageMaskShape_DEFINED
