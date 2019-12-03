/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkYUVAIndex.h"
#include "include/core/SkYUVASizeInfo.h"
#include "src/core/SkCachedData.h"
#include "src/image/SkImage_Base.h"
#include "tools/Resources.h"

DEF_SIMPLE_GM(jpg_gbr_yuv, canvas, 275, (3 * 207)) {
    auto image = GetResourceAsImage("images/icc-v2-gbr.jpg");
    auto rgbImage = image->makeRasterImage();
    canvas->drawImage(rgbImage, 0, 0);
    canvas->translate(0, image->height());
    SkYUVASizeInfo sinfo;
    SkYUVAIndex indices[4];
    SkYUVColorSpace colorSpace;
    const void* planes[4];
    if (!as_IB(image)->getPlanes(&sinfo, indices, &colorSpace, planes)) {
        return;
    }
    SkPixmap pixmap[4];
    for (int i = 0; i < 4; ++i) {
        if (!sinfo.fSizes[i].isEmpty()) {
            pixmap[i].reset(SkImageInfo::MakeA8(sinfo.fSizes[i]), planes[i], sinfo.fWidthBytes[i]);
        }
    }
    if (auto image2 = SkImage::MakeFromYUVAPixmaps(
                canvas->getGrContext(), colorSpace, pixmap, indices, image->dimensions(),
                kTopLeft_GrSurfaceOrigin, false, false, image->refColorSpace())) {
        canvas->drawImage(image2, 0, 0);
        canvas->translate(0, image->height());
        SkPaint paint;
        paint.setBlendMode(SkBlendMode::kSrc);
        canvas->drawImage(rgbImage, 0, 0, &paint);
        paint.setBlendMode(SkBlendMode::kDifference);
        canvas->drawImage(image2, 0, 0, &paint);
    }

}
