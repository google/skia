/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTiledImageUtils.h"

namespace SkTiledImageUtils {

void DrawImageRect(SkCanvas* canvas,
                   const SkImage* image,
                   const SkRect& src,
                   const SkRect& dst,
                   const SkSamplingOptions& sampling,
                   const SkPaint* paint,
                   SkCanvas::SrcRectConstraint constraint) {
    if (!image) {
        return;
    }

    if (canvas->recordingContext() || canvas->recorder()) {
        // TODO: branch off into Ganesh and Graphite specific tiling
    }

    canvas->drawImageRect(image, src, dst, sampling, paint, constraint);
}

} // namespace SkTiledImageUtils
