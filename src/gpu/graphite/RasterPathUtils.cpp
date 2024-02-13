/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/RasterPathUtils.h"

#include "include/core/SkStrokeRec.h"
#include "include/private/base/SkFixed.h"
#include "src/core/SkBlitter_A8.h"
#include "src/gpu/graphite/geom/Shape.h"
#include "src/gpu/graphite/geom/Transform_graphite.h"

namespace skgpu::graphite {

bool RasterMaskHelper::init(SkISize pixmapSize) {
    if (!fPixels) {
        return false;
    }

    // Allocate pixmap if needed
    if (!fPixels->addr()) {
        const SkImageInfo bmImageInfo = SkImageInfo::MakeA8(pixmapSize);
        if (!fPixels->tryAlloc(bmImageInfo)) {
            return false;
        }
        fPixels->erase(0);
    } else if (fPixels->dimensions() != pixmapSize) {
        return false;
    }

    fDraw.fBlitterChooser = SkA8Blitter_Choose;
    fDraw.fDst      = *fPixels;
    fDraw.fRC       = &fRasterClip;
    return true;
}

void RasterMaskHelper::drawShape(const Shape& shape,
                                 const Transform& transform,
                                 const SkStrokeRec& strokeRec,
                                 const SkIRect& resultBounds) {
    fRasterClip.setRect(resultBounds);

    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);  // "Replace" mode
    paint.setAntiAlias(true);
    // SkPaint's color is unpremul so this will produce alpha in every channel.
    paint.setColor(SK_ColorWHITE);
    strokeRec.applyToPaint(&paint);

    SkMatrix translatedMatrix = SkMatrix(transform);
    // The atlas transform of the shape is the linear-components (scale, rotation, skew) of
    // `localToDevice` translated by the top-left offset of the resultBounds.
    // We will need to translate draws so the bound's UL corner is at the origin
    translatedMatrix.postTranslate(resultBounds.x(), resultBounds.y());

    fDraw.fCTM = &translatedMatrix;
    SkPath path = shape.asPath();
    if (path.isInverseFillType()) {
        // The shader will handle the inverse fill in this case
        path.toggleInverseFillType();
    }
    fDraw.drawPathCoverage(path, paint);
}

skgpu::UniqueKey GeneratePathMaskKey(const Shape& shape,
                                     const Transform& transform,
                                     const SkStrokeRec& strokeRec,
                                     skvx::half2 maskSize) {
    skgpu::UniqueKey maskKey;
    {
        static const skgpu::UniqueKey::Domain kDomain = skgpu::UniqueKey::GenerateDomain();
        skgpu::UniqueKey::Builder builder(&maskKey, kDomain, 6 + shape.keySize(),
                                          "Raster Path Mask");
        builder[0] = maskSize.x() | (maskSize.y() << 16);

        // We require the upper left 2x2 of the matrix to match exactly for a cache hit.
        SkMatrix mat = transform.matrix().asM33();
        SkScalar sx = mat.get(SkMatrix::kMScaleX);
        SkScalar sy = mat.get(SkMatrix::kMScaleY);
        SkScalar kx = mat.get(SkMatrix::kMSkewX);
        SkScalar ky = mat.get(SkMatrix::kMSkewY);
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
        // Fractional translate does not affect caching on Android. This is done for better cache
        // hit ratio and speed and is matching HWUI behavior, which didn't consider the matrix
        // at all when caching paths.
        SkFixed fracX = 0;
        SkFixed fracY = 0;
#else
        SkScalar tx = mat.get(SkMatrix::kMTransX);
        SkScalar ty = mat.get(SkMatrix::kMTransY);
        // Allow 8 bits each in x and y of subpixel positioning.
        SkFixed fracX = SkScalarToFixed(SkScalarFraction(tx)) & 0x0000FF00;
        SkFixed fracY = SkScalarToFixed(SkScalarFraction(ty)) & 0x0000FF00;
#endif
        builder[1] = SkFloat2Bits(sx);
        builder[2] = SkFloat2Bits(sy);
        builder[3] = SkFloat2Bits(kx);
        builder[4] = SkFloat2Bits(ky);
        // FracX and fracY are &ed with 0x0000ff00, so need to shift one down to fill 16 bits.
        uint32_t fracBits = fracX | (fracY >> 8);
        // Distinguish between hairline and filled paths. For hairlines, we also need to include
        // the cap. (SW grows hairlines by 0.5 pixel with round and square caps). Note that
        // stroke-and-fill of hairlines is turned into pure fill by SkStrokeRec, so this covers
        // all cases we might see.
        uint32_t styleBits = strokeRec.isHairlineStyle() ? ((strokeRec.getCap() << 1) | 1) : 0;
        builder[5] = fracBits | (styleBits << 16);
        shape.writeKey(&builder[6], /*includeInverted=*/false);
    }
    return maskKey;
}

}  // namespace skgpu::graphite
