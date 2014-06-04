/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGatherPixelRefsAndRects.h"
#include "SkNoSaveLayerCanvas.h"
#include "SkPictureUtils.h"

void SkPictureUtils::GatherPixelRefsAndRects(SkPicture* pict,
                                             SkPictureUtils::SkPixelRefContainer* prCont) {
    if (0 == pict->width() || 0 == pict->height()) {
        return ;
    }

    SkGatherPixelRefsAndRectsDevice device(pict->width(), pict->height(), prCont);
    SkNoSaveLayerCanvas canvas(&device);

    canvas.clipRect(SkRect::MakeWH(SkIntToScalar(pict->width()),
                                   SkIntToScalar(pict->height())),
                    SkRegion::kIntersect_Op, false);
    canvas.drawPicture(pict);
}
