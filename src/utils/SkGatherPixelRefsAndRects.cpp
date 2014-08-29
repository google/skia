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
    if (pict->cullRect().isEmpty()) {
        return ;
    }

    SkGatherPixelRefsAndRectsDevice device(SkScalarCeilToInt(pict->cullRect().width()), 
                                           SkScalarCeilToInt(pict->cullRect().height()), 
                                           prCont);
    SkNoSaveLayerCanvas canvas(&device);

    canvas.clipRect(pict->cullRect(), SkRegion::kIntersect_Op, false);
    canvas.drawPicture(pict);
}
