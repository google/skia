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

/** Checks the given VerifierResult. If it is not ok, returns it. */
#define RETURN_NOT_OK(res)  if (!(res).ok()) return (res)

namespace skiagm {
namespace verifiers {

void RelaxNearEdges::init(const SkBitmap& goldBmp) {
    fBitmap.reset(new SkBitmap);
    detectEdges(goldBmp, fBitmap.get());
}

bool RelaxNearEdges::relaxPixel(int x, int y) {
    // return true if the pixel is near an edge.

    const SkIRect bounds = fBitmap->bounds();
    const int minX = std::max(x - fPixelRadius, bounds.fLeft),
        maxX = std::min(x + fPixelRadius, bounds.fRight - 1),
        minY = std::max(y - fPixelRadius, bounds.fTop),
        maxY = std::min(y + fPixelRadius, bounds.fBottom - 1);

    for (int i = minY; i <= maxY; i++) {
        for (int j = minX; j <= maxX; j++) {
            if (utils::maxChannelDiff(fBitmap->getColor(j, i), SK_ColorWHITE)
                <= kMaxDiffFromWhite) {
                return true;
            }
        }
    }

    return false;
}

void RelaxNearEdges::detectEdges(const SkBitmap& inputBmp, SkBitmap* resultBmp) {
    const auto makeFilter = [&inputBmp](const SkScalar* kernel) {
        return SkImageFilters::MatrixConvolution(
            SkISize::Make(3, 3), kernel, 1, 0, {1, 1}, SkTileMode::kRepeat, false,
            SkImageFilters::Image(SkImage::MakeFromBitmap(inputBmp)));
    };

    // Two-stage Sobel edge detection kernel
    SkBitmap imgX;
    imgX.allocPixelsFlags(inputBmp.info(), SkBitmap::kZeroPixels_AllocFlag);
    {
        const SkScalar k_gx[] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
        SkCanvas canvas(imgX);
        SkPaint paint;
        paint.setImageFilter(makeFilter(k_gx));
        canvas.drawBitmap(inputBmp, 0, 0, &paint);
    }

    SkBitmap imgY;
    imgY.allocPixelsFlags(inputBmp.info(), SkBitmap::kZeroPixels_AllocFlag);
    {
        const SkScalar k_gy[] = {-1, -2, -1, 0, 0, 0, 1, 2, 1};
        SkCanvas canvas(imgY);
        SkPaint paint;
        paint.setImageFilter(makeFilter(k_gy));
        canvas.drawBitmap(inputBmp, 0, 0, &paint);
    }

    resultBmp->allocPixelsFlags(inputBmp.info(), SkBitmap::kZeroPixels_AllocFlag);
    for (int y = 0; y < resultBmp->height(); y++) {
        for (int x = 0; x < resultBmp->width(); x++) {
            const SkColor cx = imgX.getColor(x, y), cy = imgY.getColor(x, y);
            const auto r = (uint32_t)sqrt(
                (double)SkColorGetR(cx) * SkColorGetR(cx) + SkColorGetR(cy) * SkColorGetR(cy));
            const auto g = (uint32_t)sqrt(
                (double)SkColorGetG(cx) * SkColorGetG(cx) + SkColorGetG(cy) * SkColorGetG(cy));
            const auto b = (uint32_t)sqrt(
                (double)SkColorGetB(cx) * SkColorGetB(cx) + SkColorGetB(cy) * SkColorGetB(cy));
            const uint8_t m = (uint8_t)std::min(std::max(r, std::max(g, b)), (uint32_t)0xff);
            SkColor c = SkColorSetARGB(0xff, m, m, m);
            resultBmp->erase(c, SkIRect::MakeLTRB(x, y, x + 1, y + 1));
        }
    }
}
RelaxNearEdges::RelaxNearEdges(int pixelRadius) : fPixelRadius(pixelRadius) {}

ExactPixelMatch::ExactPixelMatch(float percentPixelDifferenceThreshold) :
    fPercentPixelDifferenceThreshold(percentPixelDifferenceThreshold) {}

SkString ExactPixelMatch::name() const {
    return SkString("ExactPixelMatch");
}

VerifierResult ExactPixelMatch::verifyImpl(
    const SkIRect& region, const SkBitmap& gold, const SkBitmap& actual) {
    fDebugImage = std::make_unique<SkBitmap>();
    *fDebugImage = utils::duplicateBitmap(actual);

    int numFailedPixels = 0;
    for (int y = region.fTop; y < region.fBottom; y++) {
        for (int x = region.fLeft; x < region.fRight; x++) {
            if (gold.getColor(x, y) != actual.getColor(x, y) && !relaxPixel(x, y)) {
                numFailedPixels++;
                fDebugImage->erase(SK_ColorRED, SkIRect::MakeLTRB(x, y, x + 1, y + 1));
            }
        }
    }

    const int failedPixelThreshold =
        static_cast<int>(fPercentPixelDifferenceThreshold * region.size().area());
    return numFailedPixels <= failedPixelThreshold ? VerifierResult::Ok() : makeError(
        SkStringPrintf(
            "%d failed pixels is above threshold of %d", numFailedPixels, failedPixelThreshold));
}

CompareToMask::CompareToMask(
    SkColor backgroundColor, int backgroundColorDistanceThreshold,
    float percentPixelDifferenceThreshold) :
    fBackgroundColor(backgroundColor), fBackgroundColorDistanceThreshold(
    backgroundColorDistanceThreshold), fPercentPixelDifferenceThreshold(
    percentPixelDifferenceThreshold) {}

SkString CompareToMask::name() const {
    return SkString("CompareToMask");
}

VerifierResult CompareToMask::verifyImpl(
    const SkIRect& region, const SkBitmap& gold, const SkBitmap& actual) {
    fDebugImage = std::make_unique<SkBitmap>();
    *fDebugImage = utils::duplicateBitmap(actual);

    auto masked = makeMask(gold);
    int numFailedPixels = 0;
    for (int y = region.fTop; y < region.fBottom; y++) {
        for (int x = region.fLeft; x < region.fRight; x++) {
            const SkColor c = actual.getColor(x, y);
            const int distanceFromBG = utils::colorDist(c, fBackgroundColor);
            if (distanceFromBG > fBackgroundColorDistanceThreshold
                && !utils::colorInNeighborhood(masked, x, y, SK_ColorBLACK) && !relaxPixel(x, y)) {
                // Test image drew a pixel that gold image did not (approximately).
                numFailedPixels++;
                fDebugImage->erase(SK_ColorRED, SkIRect::MakeLTRB(x, y, x + 1, y + 1));
            }
        }
    }

    const int failedPixelThreshold =
        static_cast<int>(fPercentPixelDifferenceThreshold * region.size().area());
    return numFailedPixels <= failedPixelThreshold ? VerifierResult::Ok() : makeError(
        SkStringPrintf(
            "%d failed pixels is above threshold of %d", numFailedPixels, failedPixelThreshold));
}

SkBitmap CompareToMask::makeMask(const SkBitmap& bmp) {
    const SkISize inputSize = bmp.info().bounds().size();
    const SkImageInfo
        imageInfo = SkImageInfo::Make(inputSize, bmp.colorType(), bmp.alphaType(), nullptr);

    SkBitmap result;
    result.allocPixelsFlags(imageInfo, SkBitmap::kZeroPixels_AllocFlag);

    SkCanvas canvas(result);
    canvas.clear(SK_ColorWHITE);
    for (int y = 0; y < inputSize.fHeight; y++) {
        for (int x = 0; x < inputSize.fWidth; x++) {
            SkColor c = bmp.getColor(x, y);
            if (c != SK_ColorWHITE && SkColorGetA(c) > 0) {
                result.erase(SK_ColorBLACK, SkIRect::MakeLTRB(x, y, x + 1, y + 1));
            }
        }
    }

    return result;
}

CheckPixelColorNearby::CheckPixelColorNearby(
    int pixelRadius, int colorDistanceThreshold, float percentPixelDifferenceThreshold) :
    fPixelRadius(pixelRadius),
    fColorDistanceThreshold(colorDistanceThreshold),
    fPercentPixelDifferenceThreshold(percentPixelDifferenceThreshold) {}

SkString CheckPixelColorNearby::name() const {
    return SkString("CheckPixelColorNearby");
}

VerifierResult CheckPixelColorNearby::verifyImpl(
    const SkIRect& region, const SkBitmap& gold, const SkBitmap& actual) {
    fDebugImage = std::make_unique<SkBitmap>();
    *fDebugImage = utils::duplicateBitmap(actual);

    int numFailedPixels = 0;
    for (int y = region.fTop; y < region.fBottom; y++) {
        for (int x = region.fLeft; x < region.fRight; x++) {
            const SkColor c = actual.getColor(x, y);
            if (!utils::colorInNeighborhood(
                gold, x, y, c, fPixelRadius, fColorDistanceThreshold) && !relaxPixel(x, y)) {
                // Test image drew a pixel that gold image did not (approximately).
                numFailedPixels++;
                fDebugImage->erase(SK_ColorRED, SkIRect::MakeLTRB(x, y, x + 1, y + 1));
            }
        }
    }

    const int failedPixelThreshold =
        static_cast<int>(fPercentPixelDifferenceThreshold * region.size().area());
    return numFailedPixels <= failedPixelThreshold ? VerifierResult::Ok() : makeError(
        SkStringPrintf(
            "%d failed pixels is above threshold of %d", numFailedPixels, failedPixelThreshold));
}

CheckNoPixelsOutsideRegion::CheckNoPixelsOutsideRegion(SkColor backgroundColor) : fBackgroundColor(
    backgroundColor), fInverted(false) {
}

SkString CheckNoPixelsOutsideRegion::name() const {
    return SkString("CheckNoPixelsOutsideRegion");
}

void CheckNoPixelsOutsideRegion::toggleInverted() {
    fInverted = !fInverted;
}

VerifierResult CheckNoPixelsOutsideRegion::verifyImpl(
    const SkIRect& region, const SkBitmap& gold, const SkBitmap& actual) {
    fDebugImage = std::make_unique<SkBitmap>();
    *fDebugImage = utils::duplicateBitmap(actual);

    int numFailedPixels = 0;
    if (fInverted) {
        // No pixels should appear within the region.
        for (int y = region.fTop; y < region.fBottom; y++) {
            for (int x = region.fLeft; x < region.fRight; x++) {
                const SkColor c = actual.getColor(x, y);
                if (c != fBackgroundColor && !relaxPixel(x, y)) {
                    numFailedPixels++;
                    fDebugImage->erase(SK_ColorRED, SkIRect::MakeLTRB(x, y, x + 1, y + 1));
                }
            }
        }
    } else {
        // No pixels should appear outside the region.
        for (int y = 0; y < actual.bounds().fBottom; y++) {
            for (int x = 0; x < actual.bounds().fRight; x++) {
                const SkColor c = actual.getColor(x, y);
                if (!region.contains(x, y) && c != fBackgroundColor && !relaxPixel(x, y)) {
                    numFailedPixels++;
                    fDebugImage->erase(SK_ColorRED, SkIRect::MakeLTRB(x, y, x + 1, y + 1));
                }
            }
        }
    }

    if (numFailedPixels == 0) {
        return VerifierResult::Ok();
    } else {
        return makeError(
            SkStringPrintf(
                "%d pixels drawn %s region (%s)", numFailedPixels, fInverted ? "inside" : "outside",
                utils::toString(region).c_str()));
    }
}

}
}
