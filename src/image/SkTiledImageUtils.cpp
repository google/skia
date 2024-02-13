/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTiledImageUtils.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPixelRef.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTFitsIn.h"
#include "src/core/SkCanvasPriv.h"
#include "src/core/SkDevice.h"
#include "src/image/SkImage_Base.h"
#include "src/image/SkImage_Picture.h"

#include <string.h>

namespace SkTiledImageUtils {

void DrawImageRect(SkCanvas* canvas,
                   const SkImage* image,
                   const SkRect& src,
                   const SkRect& dst,
                   const SkSamplingOptions& sampling,
                   const SkPaint* paint,
                   SkCanvas::SrcRectConstraint constraint) {
    if (!image || !canvas) {
        return;
    }

    SkPaint p;
    if (paint) {
        p = *paint;
    }
    if (!SkCanvasPriv::TopDevice(canvas)->drawAsTiledImageRect(
                canvas, image, &src, dst, sampling, p, constraint)) {
        // Either the image didn't require tiling or this is a raster-backed
        // canvas. In either case fall back to a default draw.
        canvas->drawImageRect(image, src, dst, sampling, paint, constraint);
    }
}

void GetImageKeyValues(const SkImage* image, uint32_t keyValues[kNumImageKeyValues]) {
    if (!image || !keyValues) {
        if (keyValues) {
            memset(keyValues, 0, kNumImageKeyValues * sizeof(uint32_t));
        }
        return;
    }

    const SkImage_Base* imageBase = as_IB(image);
    if (const SkBitmap* bm = imageBase->onPeekBitmap()) {
        keyValues[0] = bm->pixelRef()->getGenerationID();
        SkIRect subset = image->bounds();
        subset.offset(bm->pixelRefOrigin());

        SkASSERT(SkTFitsIn<uint32_t>(subset.fLeft));
        SkASSERT(SkTFitsIn<uint32_t>(subset.fTop));
        SkASSERT(SkTFitsIn<uint32_t>(subset.fRight));
        SkASSERT(SkTFitsIn<uint32_t>(subset.fBottom));

        keyValues[1] = 0;              // This empty slot is to disambiguate picture IDs
        keyValues[2] = subset.fLeft;
        keyValues[3] = subset.fTop;
        keyValues[4] = subset.fRight;
        keyValues[5] = subset.fBottom;
        return;
    }

    if (imageBase->type() == SkImage_Base::Type::kLazyPicture) {
        const SkImage_Picture* pictureImage = static_cast<const SkImage_Picture*>(imageBase);
        if (pictureImage->getImageKeyValues(keyValues)) {
            return;
        }
    }

    keyValues[0] = image->uniqueID();
    memset(&keyValues[1], 0, (kNumImageKeyValues-1) * sizeof(uint32_t));
}

} // namespace SkTiledImageUtils
