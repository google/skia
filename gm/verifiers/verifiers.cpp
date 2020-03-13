/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/verifiers/utils.h"
#include "gm/verifiers/verifiers.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/effects/SkImageFilters.h"

namespace skiagm {
namespace verifiers {

CheckPixelColorNearby::CheckPixelColorNearby(
    int pixelRadius, float colorDistanceThreshold, float percentPixelDifferenceThreshold) :
    fPixelRadius(pixelRadius),
    fColorDistanceThreshold(colorDistanceThreshold),
    fPercentPixelDifferenceThreshold(percentPixelDifferenceThreshold) {}

SkString CheckPixelColorNearby::name() const {
    return SkString("CheckPixelColorNearby");
}

VerifierResult CheckPixelColorNearby::verifyWithGold(
    const SkIRect& region, const SkBitmap& gold, const SkBitmap& actual) {
    int numFailedPixels = 0;
    for (int y = region.fTop; y < region.fBottom; y++) {
        for (int x = region.fLeft; x < region.fRight; x++) {
            const SkColor4f c = utils::getColor(actual, x, y);
            if (!utils::colorInNeighborhood(gold, x, y, c, fPixelRadius, fColorDistanceThreshold)) {
                // Test image drew a pixel that gold image did not (approximately).
                numFailedPixels++;
            }
        }
    }

    const int failedPixelThreshold =
        static_cast<int>(fPercentPixelDifferenceThreshold * region.size().area());
    return numFailedPixels <= failedPixelThreshold ? VerifierResult::Ok() : makeError(
        SkStringPrintf(
            "%d failed pixels is above threshold of %d", numFailedPixels, failedPixelThreshold));
}

}
}
