/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTiledImageUtils.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkPixelRef.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTFitsIn.h"
#include "src/image/SkImage_Base.h"

#if defined(SK_GRAPHITE)
#include "src/gpu/TiledTextureUtils.h"
#endif

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

#if defined(SK_GRAPHITE)
    if (canvas->recorder()) {
        if (skgpu::TiledTextureUtils::DrawAsTiledImageRect(canvas, image, src, dst,
                                                           SkCanvas::kAll_QuadAAFlags, sampling,
                                                           paint, constraint)) {
            return;
        }
    }
#endif

    canvas->drawImageRect(image, src, dst, sampling, paint, constraint);
}

void GetImageKeyValues(const SkImage* image, uint32_t keyValues[kNumImageKeyValues]) {
    if (!image || !keyValues) {
        if (keyValues) {
            memset(keyValues, 0, kNumImageKeyValues * sizeof(uint32_t));
        }
        return;
    }

    SkIRect subset = image->bounds();

    if (const SkBitmap* bm = as_IB(image)->onPeekBitmap()) {
        keyValues[0] = bm->pixelRef()->getGenerationID();
        subset.offset(bm->pixelRefOrigin());
    } else {
        keyValues[0] = image->uniqueID();
    }

    SkASSERT(SkTFitsIn<uint32_t>(subset.fLeft));
    SkASSERT(SkTFitsIn<uint32_t>(subset.fTop));
    SkASSERT(SkTFitsIn<uint32_t>(subset.fRight));
    SkASSERT(SkTFitsIn<uint32_t>(subset.fBottom));

    keyValues[1] = subset.fLeft;
    keyValues[2] = subset.fTop;
    keyValues[3] = subset.fRight;
    keyValues[4] = subset.fBottom;
}

} // namespace SkTiledImageUtils
