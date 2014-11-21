/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImagePriv.h"
#include "SkImage_Base.h"
#include "SkCanvas.h"
#include "SkPicture.h"

SkImage* SkNewImageFromBitmap(const SkBitmap& bm, bool canSharePixelRef,
                              const SkSurfaceProps* props) {
    const SkImageInfo info = bm.info();
    if (kUnknown_SkColorType == info.colorType()) {
        return NULL;
    }

    SkImage* image = NULL;
    if (canSharePixelRef || bm.isImmutable()) {
        image = SkNewImageFromPixelRef(info, bm.pixelRef(), bm.rowBytes(), props);
    } else {
        bm.lockPixels();
        if (bm.getPixels()) {
            image = SkImage::NewRasterCopy(info, bm.getPixels(), bm.rowBytes());
        }
        bm.unlockPixels();

        // we don't expose props to NewRasterCopy (need a private vers) so post-init it here
        if (image && props) {
            as_IB(image)->initWithProps(*props);
        }
    }
    return image;
}
