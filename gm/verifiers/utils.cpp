/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkRect.h"
#include "include/core/SkString.h"

namespace skiagm {
namespace verifiers {
namespace utils {

namespace {
/** Returns |x-y| */
uint8_t absDelta(uint8_t x, uint8_t y) {
    return x < y ? y - x : x - y;
}
}

int colorDist(SkColor a, SkColor b) {
    const uint8_t a_a = SkColorGetA(a), a_r = SkColorGetR(a), a_g = SkColorGetG(a),
        a_b = SkColorGetB(a);
    const uint8_t b_a = SkColorGetA(b), b_r = SkColorGetR(b), b_g = SkColorGetG(b),
        b_b = SkColorGetB(b);

    const uint32_t da = absDelta(a_a, b_a);
    const uint32_t dr = absDelta(a_r, b_r);
    const uint32_t dg = absDelta(a_g, b_g);
    const uint32_t db = absDelta(a_b, b_b);

    return da + dr + dg + db;
}

int maxChannelDiff(SkColor a, SkColor b) {
    const uint8_t a_a = SkColorGetA(a), a_r = SkColorGetR(a), a_g = SkColorGetG(a),
        a_b = SkColorGetB(a);
    const uint8_t b_a = SkColorGetA(b), b_r = SkColorGetR(b), b_g = SkColorGetG(b),
        b_b = SkColorGetB(b);

    const uint32_t da = absDelta(a_a, b_a);
    const uint32_t dr = absDelta(a_r, b_r);
    const uint32_t dg = absDelta(a_g, b_g);
    const uint32_t db = absDelta(a_b, b_b);

    return std::max(std::max(std::max(da, dr), dg), db);
}

bool colorInNeighborhood(
    const SkBitmap& bitmap, int x, int y, SkColor color, int n, int dist) {
    const SkIRect bounds = bitmap.bounds();
    const int minX = std::max(x - n, bounds.fLeft), maxX = std::min(x + n, bounds.fRight - 1),
        minY = std::max(y - n, bounds.fTop), maxY = std::min(y + n, bounds.fBottom - 1);

    for (int i = minY; i <= maxY; i++) {
        for (int j = minX; j <= maxX; j++) {
            if (maxChannelDiff(color, bitmap.getColor(j, i)) <= dist) {
                return true;
            }
        }
    }

    return false;
}

SkString toString(const SkIRect& r) {
    return SkStringPrintf("%d, %d, %d, %d", r.fLeft, r.fTop, r.fRight, r.fBottom);
}

SkString toString(const SkColor& c) {
    return SkStringPrintf(
        "ARGB 0x%0.2x%0.2x%0.2x%.02x", SkColorGetA(c), SkColorGetR(c), SkColorGetG(c),
        SkColorGetB(c));
}

SkBitmap duplicateBitmap(const SkBitmap& bmp) {
    SkBitmap result;
    result.allocPixelsFlags(bmp.info(), SkBitmap::kZeroPixels_AllocFlag);

    for (int y = 0; y < result.height(); y++) {
        for (int x = 0; x < result.width(); x++) {
            result.erase(bmp.getColor(x, y), SkIRect::MakeLTRB(x, y, x + 1, y + 1));
        }
    }

    return result;
}

}
}
}
