/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkWritePixelsRec.h"

#include "include/core/SkRect.h"
#include "src/base/SkSafeMath.h"

bool SkWritePixelsRec::trim(int dstWidth, int dstHeight) {
    // fInfo.minRowBytes() returns 0 if the size doesn't fit in `size_t`.
    const size_t minRowBytes = fInfo.minRowBytes();
    if (nullptr == fPixels || fRowBytes < minRowBytes || minRowBytes == 0) {
        return false;
    }
    if (0 >= fInfo.width() || 0 >= fInfo.height()) {
        return false;
    }
    // negating the largest negative integer (below) is UB
    if (fX == INT_MIN || fY == INT_MIN) {
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
    // here x,y are either 0 or negative (safe to cast to size_t)
    // we negate and add them so UBSAN (pointer-overflow) doesn't get confused.
    SkSafeMath safeMath;
    const size_t y_offset = safeMath.mul(-y, fRowBytes);
    const size_t x_offset = safeMath.mul(-x, fInfo.bytesPerPixel());
    const size_t total = safeMath.add(y_offset, x_offset);
    if (!safeMath.ok()) {
        return false;
    }

    fPixels = ((const char*)fPixels + total);
    // the intersect may have shrunk info's logical size
    fInfo = fInfo.makeDimensions(dstR.size());
    fX = dstR.x();
    fY = dstR.y();

    return true;
}
