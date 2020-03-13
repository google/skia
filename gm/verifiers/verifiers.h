/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef gmverifiers_DEFINED
#define gmverifiers_DEFINED

#include "gm/verifiers/gmverifier.h"
#include "include/core/SkColor.h"
#include "include/core/SkRect.h"
#include "include/core/SkString.h"

#include <vector>

namespace skiagm {
namespace verifiers {

/**
 * Verifier that checks that pixels appearing in the actual image appear nearby (and within
 * some small color distance delta) in the gold image.
 */
class CheckPixelColorNearby : public GoldImageVerifier {
public:
    explicit CheckPixelColorNearby(
        int pixelRadius = 1, float colorDistanceThreshold = 0.07f,
        float percentPixelDifferenceThreshold = 0.01f);

    SkString name() const override;

protected:
    VerifierResult verifyWithGold(
        const SkIRect& region, const SkBitmap& gold, const SkBitmap& actual) override;

private:
    /**
     * Pixel radius to search for color match. A radius of 1 means a 3x3 region is searched in the
     * gold image, with the pixel in question in the center.
     */
    const int fPixelRadius;

    /** Distance threshold used to compare pixel color values for equality. */
    const float fColorDistanceThreshold;

    /** Acceptable percentage of pixels that can be different from the gold image. */
    const float fPercentPixelDifferenceThreshold;
};

}
}

#endif
