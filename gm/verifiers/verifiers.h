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

class SkBitmap;

namespace skiagm {
namespace verifiers {

/**
 * A verifier relaxation that allows for pixels near detected edges to be omitted from the failed
 * pixel count.
 */
class RelaxNearEdges : public VerifierRelaxation {
public:
    explicit RelaxNearEdges(int pixelRadius = 2);

    /**
     * Initializes the instance by running edge detection on the given image and storing the
     * resulting image.
     */
    void init(const SkBitmap& goldBmp) override;

    /** Returns true if the pixel at (x,y) is near a detected edge. */
    bool relaxPixel(int x, int y) override;

private:
    /** Pixel radius to search for nearby edges. */
    const int fPixelRadius;

    /** Max channel delta from 0xffffffff to be considered "white" in the edge mask. */
    const int kMaxDiffFromWhite = 8;

    /** Runs edge detection on the given input image, storing the result in the output parameter. */
    static void detectEdges(const SkBitmap& inputBmp, SkBitmap* resultBmp);
};

/** Verifier that performs an exact per-pixel comparison against the gold image. */
class ExactPixelMatch : public GMVerifier {
public:
    explicit ExactPixelMatch(float percentPixelDifferenceThreshold = 0.00f);

    SkString name() const override;

protected:
    VerifierResult verifyImpl(
        const SkIRect& region, const SkBitmap& gold, const SkBitmap& actual) override;

private:
    /** Acceptable percentage of pixels that can be different. */
    const float fPercentPixelDifferenceThreshold;
};

/**
 * Verifier that checks that pixels appearing in the actual image were present in the gold
 * image, ignoring color and discarding "mostly invisible" pixels, i.e. pixels that are very close
 * to the background color.
 */
class CompareToMask : public GMVerifier {
public:
    explicit CompareToMask(
        SkColor backgroundColor = SK_ColorWHITE, int backgroundColorDistanceThreshold = 0,
        float percentPixelDifferenceThreshold = 0.01f);

    SkString name() const override;

protected:
    VerifierResult verifyImpl(
        const SkIRect& region, const SkBitmap& gold, const SkBitmap& actual) override;

private:
    /** Background color used to determine which pixels are "mostly invisible." */
    const SkColor fBackgroundColor;

    /**
     * Distance threshold used to determine which pixels are "mostly invisible." Pixels that are
     * within this distance of the background color are considered invisible, and are not checked
     * against the mask.
     */
    const int fBackgroundColorDistanceThreshold;

    /** Acceptable percentage of pixels that can be different from the mask. */
    const float fPercentPixelDifferenceThreshold;

    /** Creates a "mask" of the given bitmap. */
    static SkBitmap makeMask(const SkBitmap& bmp);
};

/**
 * Verifier that checks that pixels appearing in the actual image appear nearby (and within
 * some small color distance delta) in the gold image.
 */
class CheckPixelColorNearby : public GMVerifier {
public:
    explicit CheckPixelColorNearby(
        int pixelRadius = 1, int colorDistanceThreshold = 16,
        float percentPixelDifferenceThreshold = 0.01f);

    SkString name() const override;

protected:
    VerifierResult verifyImpl(
        const SkIRect& region, const SkBitmap& gold, const SkBitmap& actual) override;

private:
    /**
     * Pixel radius to search for color match. A radius of 1 means a 3x3 region is searched in the
     * gold image, with the pixel in question in the center.
     */
    const int fPixelRadius;

    /** Distance threshold used to compare pixel color values for equality. */
    const int fColorDistanceThreshold;

    /** Acceptable percentage of pixels that can be different from the gold image. */
    const float fPercentPixelDifferenceThreshold;
};

/**
 * Verifier that checks that no pixels appear outside of the added region(s). If 'inverted',
 * instead checks that no pixels appear within the added region(s).
 */
class CheckNoPixelsOutsideRegion : public GMVerifier {
public:
    explicit CheckNoPixelsOutsideRegion(SkColor backgroundColor = SK_ColorWHITE);

    SkString name() const override;

    /** Inverts the added region(s). */
    void toggleInverted();

protected:
    VerifierResult verifyImpl(
        const SkIRect& region, const SkBitmap& gold, const SkBitmap& actual) override;

private:
    /** Background color used to determine whether pixels were drawn or not. */
    const SkColor fBackgroundColor;

    /** Inverted state. */
    bool fInverted;
};

}
}

#endif
