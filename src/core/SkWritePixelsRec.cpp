/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkWritePixelsRec.h"

#include "include/core/SkRect.h"

bool SkWritePixelsRec::trim(int dstWidth, int dstHeight) {
    if (nullptr == fPixels || fRowBytes < fInfo.minRowBytes()) {
        return false;
    }
    if (0 >= fInfo.width() || 0 >= fInfo.height()) {
        return false;
    }

    int x = fX;
    int y = fY;
    SkIRect dstR = SkIRect::MakeXYWH(x, y, fInfo.width(), fInfo.height());
    if (!dstR.intersect({0, 0, dstWidth, dstHeight})) {
        return false;
    }

    // if x or y are negative, then we have to adjust pixels
    if (x > 0) {
        x = 0;
    }
    if (y > 0) {
        y = 0;
    }
    // here x,y are either 0 or negative
    // we negate and add them so UBSAN (pointer-overflow) doesn't get confused.
    fPixels = ((const char*)fPixels + -y*fRowBytes + -x*fInfo.bytesPerPixel());
    // the intersect may have shrunk info's logical size
    fInfo = fInfo.makeDimensions(dstR.size());
    fX = dstR.x();
    fY = dstR.y();

    return true;
}
