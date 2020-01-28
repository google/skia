/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef verifier_utils_DEFINED
#define verifier_utils_DEFINED

#include "include/core/SkColor.h"

#include <cstdint>
#include <memory>
#include <vector>

class SkBitmap;
class SkString;

namespace skiagm {
namespace verifiers {
namespace utils {

/**
 * Returns a distance between two colors.
 *
 * dist(a, b) = sum_{c in channels} |a_c - b_c|
 */
int colorDist(SkColor a, SkColor b);

/**
 * Returns the max delta across channels of the two given colors.
 *
 * dist(a, b) = max_{c in channels} |a_c - b_c|
 */
int maxChannelDiff(SkColor a, SkColor b);

/**
 * Returns true if a color is found within a pixel neighborhood of a bitmap.
 *
 * @param bitmap Bitmap to check
 * @param x X coord of pixel
 * @param y Y coord of pixel
 * @param color Color to search for
 * @param n Pixel radius around x,y to search for color.
 * @param colorDist The color distance used for fuzzy color equality. The default of 0
 *    means that the pixels much exactly match.
 * @return True if the color is found within n pixels of (x,y) in the bitmap.
 */
bool colorInNeighborhood(
    const SkBitmap& bitmap, int x, int y, SkColor color, int n = 1, int dist = 0);

/** Converts an SkIRect to a string representation. */
SkString toString(const SkIRect& r);

/** Converts an SkColor to a string representation. */
SkString toString(const SkColor& c);

/** Duplicates the given bitmap (copies all pixel values). */
SkBitmap duplicateBitmap(const SkBitmap& bmp);

}
}
}

#endif
