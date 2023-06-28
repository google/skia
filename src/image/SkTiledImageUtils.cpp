/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTiledImageUtils.h"

#if defined(SK_GRAPHITE)
#include "src/gpu/TiledTextureUtils.h"
#endif

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
        skgpu::TiledTextureUtils::DrawImageRect_Graphite(canvas, image, src, dst, sampling, paint,
                                                         constraint);
        return;
    }
#endif

    canvas->drawImageRect(image, src, dst, sampling, paint, constraint);
}

} // namespace SkTiledImageUtils
