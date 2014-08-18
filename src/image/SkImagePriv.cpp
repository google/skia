/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImagePriv.h"
#include "SkCanvas.h"
#include "SkPicture.h"

SkImage* SkNewImageFromBitmap(const SkBitmap& bm, bool canSharePixelRef) {
    const SkImageInfo info = bm.info();
    if (kUnknown_SkColorType == info.colorType()) {
        return NULL;
    }

    SkImage* image = NULL;
    if (canSharePixelRef || bm.isImmutable()) {
        image = SkNewImageFromPixelRef(info, bm.pixelRef(), bm.rowBytes());
    } else {
        bm.lockPixels();
        if (bm.getPixels()) {
            image = SkImage::NewRasterCopy(info, bm.getPixels(), bm.rowBytes());
        }
        bm.unlockPixels();
    }
    return image;
}
