/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/RasterPathUtils.h"

#include "include/core/SkStrokeRec.h"
#include "include/private/base/SkFixed.h"
#include "src/base/SkFloatBits.h"
#include "src/core/SkBlitter_A8.h"
#include "src/gpu/graphite/geom/Shape.h"
#include "src/gpu/graphite/geom/Transform_graphite.h"

namespace skgpu::graphite {

bool RasterMaskHelper::init(SkISize pixmapSize, skvx::float2 transformedMaskOffset) {
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
    fTransformedMaskOffset = transformedMaskOffset;
    return true;
}

void RasterMaskHelper::drawShape(const Shape& shape,
                                 const Transform& localToDevice,
                                 const SkStrokeRec& strokeRec,
                                 const SkIRect& resultBounds) {
    fRasterClip.setRect(resultBounds);

    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);  // "Replace" mode
    paint.setAntiAlias(true);
    // SkPaint's color is unpremul so this will produce alpha in every channel.
    paint.setColor(SK_ColorWHITE);
    strokeRec.applyToPaint(&paint);

    SkMatrix translatedMatrix = SkMatrix(localToDevice);
    // The atlas transform of the shape is `localToDevice` translated by the top-left offset of the
    // resultBounds and the inverse of the base mask transform offset for the current set of shapes.
    // We will need to translate draws so the bound's UL corner is at the origin
    translatedMatrix.postTranslate(resultBounds.x() - fTransformedMaskOffset.x(),
                                   resultBounds.y() - fTransformedMaskOffset.y());

    fDraw.fCTM = &translatedMatrix;
    // TODO: use drawRect, drawRRect, drawArc
    SkPath path = shape.asPath();
    if (path.isInverseFillType()) {
        // The shader will handle the inverse fill in this case
        path.toggleInverseFillType();
    }
    fDraw.drawPathCoverage(path, paint);
}

void RasterMaskHelper::drawClip(const Shape& shape,
                                const Transform& transform,
                                uint8_t alpha,
                                const SkIRect& resultBounds) {
    fRasterClip.setRect(resultBounds);

    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);  // "Replace" mode
    paint.setAntiAlias(true);
    // SkPaint's color is unpremul so this will produce alpha in every channel.
    paint.setColor(SkColorSetARGB(alpha, 0xFF, 0xFF, 0xFF));

    SkMatrix translatedMatrix = SkMatrix(transform);
    // The atlas transform of the shape is the linear-components (scale, rotation, skew) of
    // `localToDevice` translated by the top-left offset of the resultBounds.
    // We will need to translate draws so the bound's UL corner is at the origin
    translatedMatrix.postTranslate(resultBounds.x(), resultBounds.y());

    fDraw.fCTM = &translatedMatrix;
    // TODO: use drawRect, drawRRect, drawArc
    SkPath path = shape.asPath();
    // Because we could be combining multiple paths into one entry we don't touch
    // the inverse fill in this case.
    if (0xFF == alpha) {
        SkASSERT(0xFF == paint.getAlpha());
        fDraw.drawPathCoverage(path, paint);
    } else {
        fDraw.drawPath(path, paint, nullptr, true);
    }
}

uint32_t add_transform_key(skgpu::UniqueKey::Builder* builder,
                           int startIndex,
                           const Transform& transform) {
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
    (*builder)[startIndex + 0] = SkFloat2Bits(sx);
    (*builder)[startIndex + 1] = SkFloat2Bits(sy);
    (*builder)[startIndex + 2] = SkFloat2Bits(kx);
    (*builder)[startIndex + 3] = SkFloat2Bits(ky);
    // FracX and fracY are &ed with 0x0000ff00, so need to shift one down to fill 16 bits.
    uint32_t fracBits = fracX | (fracY >> 8);

    return fracBits;
}

skgpu::UniqueKey GeneratePathMaskKey(const Shape& shape,
                                     const Transform& transform,
                                     const SkStrokeRec& strokeRec,
                                     skvx::half2 maskOrigin,
                                     skvx::half2 maskSize) {
    skgpu::UniqueKey maskKey;
    {
        static const skgpu::UniqueKey::Domain kDomain = skgpu::UniqueKey::GenerateDomain();
        int styleKeySize = 7;
        if (!strokeRec.isHairlineStyle() && !strokeRec.isFillStyle()) {
            // Add space for width and miter if needed
            styleKeySize += 2;
        }
        skgpu::UniqueKey::Builder builder(&maskKey, kDomain, styleKeySize + shape.keySize(),
                                          "Raster Path Mask");
        builder[0] = maskOrigin.x() | (maskOrigin.y() << 16);
        builder[1] = maskSize.x() | (maskSize.y() << 16);

        // Add transform key and get packed fractional translation bits
        uint32_t fracBits = add_transform_key(&builder, 2, transform);
        // Distinguish between path styles. For anything but fill, we also need to include
        // the cap. (SW grows hairlines by 0.5 pixel with round and square caps). For stroke
        // or fill-and-stroke we need to include the join, width, and miter.
        static_assert(SkStrokeRec::kStyleCount <= (1 << 2));
        static_assert(SkPaint::kCapCount <= (1 << 2));
        static_assert(SkPaint::kJoinCount <= (1 << 2));
        uint32_t styleBits = strokeRec.getStyle();
        if (!strokeRec.isFillStyle()) {
            styleBits |= (strokeRec.getCap() << 2);
        }
        if (!strokeRec.isHairlineStyle() && !strokeRec.isFillStyle()) {
            styleBits |= (strokeRec.getJoin() << 4);
            builder[6] = SkFloat2Bits(strokeRec.getWidth());
            builder[7] = SkFloat2Bits(strokeRec.getMiter());
        }
        builder[styleKeySize-1] = fracBits | (styleBits << 16);
        shape.writeKey(&builder[styleKeySize], /*includeInverted=*/false);
    }
    return maskKey;
}

skgpu::UniqueKey GenerateClipMaskKey(uint32_t stackRecordID,
                                     const ClipStack::ElementList* elementsForMask) {
    skgpu::UniqueKey maskKey;
    {
        static constexpr int kMaxShapeCountForKey = 2;

        static const skgpu::UniqueKey::Domain kDomain = skgpu::UniqueKey::GenerateDomain();
        // if the element list is too large we just use the stackRecordID
        if (elementsForMask->size() > kMaxShapeCountForKey) {
            skgpu::UniqueKey::Builder builder(&maskKey, kDomain, 1, "Clip Path Mask");
            builder[0] = stackRecordID;
        } else {
            int xformKeySize = 5;
            int keySize = 0;
            for (int i = 0; i < elementsForMask->size(); ++i) {
                keySize += xformKeySize + (*elementsForMask)[i]->fShape.keySize();
            }
            skgpu::UniqueKey::Builder builder(&maskKey, kDomain, keySize,
                                              "Clip Path Mask");
            int elementKeyIndex = 0;
            for (int i = 0; i < elementsForMask->size(); ++i) {
                // Add transform key and get packed fractional translation bits
                uint32_t fracBits = add_transform_key(&builder,
                                                      elementKeyIndex,
                                                      (*elementsForMask)[i]->fLocalToDevice);
                uint32_t opBits = static_cast<uint32_t>((*elementsForMask)[i]->fOp);
                builder[elementKeyIndex + 4] = fracBits | (opBits << 16);

                const Shape& shape = (*elementsForMask)[i]->fShape;
                shape.writeKey(&builder[elementKeyIndex + xformKeySize], /*includeInverted=*/true);

                elementKeyIndex += xformKeySize + shape.keySize();
            }
        }

    }

    return maskKey;

}

}  // namespace skgpu::graphite
