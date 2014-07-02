/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDifferentPixelsMetric.h"

#include "SkBitmap.h"
#include "skpdiff_util.h"

const char* SkDifferentPixelsMetric::getName() const {
    return "different_pixels";
}

bool SkDifferentPixelsMetric::diff(SkBitmap* baseline, SkBitmap* test,
                                   const BitmapsToCreate& bitmapsToCreate,
                                   Result* result) const {
    double startTime = get_seconds();

    // Ensure the images are comparable
    if (baseline->width() != test->width() || baseline->height() != test->height() ||
        baseline->width() <= 0 || baseline->height() <= 0 ||
        baseline->colorType() != test->colorType()) {
        SkASSERT(baseline->width() == test->width());
        SkASSERT(baseline->height() == test->height());
        SkASSERT(baseline->width() > 0);
        SkASSERT(baseline->height() > 0);
        SkASSERT(baseline->colorType() == test->colorType());
        return false;
    }

    int width = baseline->width();
    int height = baseline->height();
    int maxRedDiff = 0;
    int maxGreenDiff = 0;
    int maxBlueDiff = 0;

    // Prepare any bitmaps we will be filling in
    if (bitmapsToCreate.alphaMask) {
        result->poiAlphaMask.allocPixels(SkImageInfo::MakeA8(width, height));
        result->poiAlphaMask.eraseARGB(SK_AlphaOPAQUE, 0, 0, 0);
    }
    if (bitmapsToCreate.rgbDiff) {
        result->rgbDiffBitmap.allocPixels(SkImageInfo::Make(width, height, baseline->colorType(),
                                                            kPremul_SkAlphaType));
        result->rgbDiffBitmap.eraseARGB(SK_AlphaTRANSPARENT, 0, 0, 0);
    }
    if (bitmapsToCreate.whiteDiff) {
        result->whiteDiffBitmap.allocPixels(SkImageInfo::MakeN32Premul(width, height));
        result->whiteDiffBitmap.eraseARGB(SK_AlphaOPAQUE, 0, 0, 0);
    }

    // Prepare the pixels for comparison
    result->poiCount = 0;
    baseline->lockPixels();
    test->lockPixels();
    for (int y = 0; y < height; y++) {
        // Grab a row from each image for easy comparison
        // TODO(epoger): The code below already assumes 4 bytes per pixel, so I think
        // we could just call getAddr32() to save a little time.
        // OR, if we want to play it safe, call ComputeBytesPerPixel instead
        // of assuming 4 bytes per pixel.
        uint32_t* baselineRow = static_cast<uint32_t *>(baseline->getAddr(0, y));
        uint32_t* testRow = static_cast<uint32_t *>(test->getAddr(0, y));
        for (int x = 0; x < width; x++) {
            // Compare one pixel at a time so each differing pixel can be noted
            // TODO(epoger): This loop looks like a good place to work on performance,
            // but we should run the code through a profiler to be sure.
            uint32_t baselinePixel = baselineRow[x];
            uint32_t testPixel = testRow[x];
            if (baselinePixel != testPixel) {
                result->poiCount++;

                int redDiff = abs(static_cast<int>(SkColorGetR(baselinePixel) -
                                                   SkColorGetR(testPixel)));
                if (redDiff > maxRedDiff) {maxRedDiff = redDiff;}
                int greenDiff = abs(static_cast<int>(SkColorGetG(baselinePixel) -
                                                     SkColorGetG(testPixel)));
                if (greenDiff > maxGreenDiff) {maxGreenDiff = greenDiff;}
                int blueDiff = abs(static_cast<int>(SkColorGetB(baselinePixel) -
                                                    SkColorGetB(testPixel)));
                if (blueDiff > maxBlueDiff) {maxBlueDiff = blueDiff;}

                if (bitmapsToCreate.alphaMask) {
                    *result->poiAlphaMask.getAddr8(x,y) = SK_AlphaTRANSPARENT;
                }
                if (bitmapsToCreate.rgbDiff) {
                    *result->rgbDiffBitmap.getAddr32(x,y) =
                        SkColorSetRGB(redDiff, greenDiff, blueDiff);
                }
                if (bitmapsToCreate.whiteDiff) {
                    *result->whiteDiffBitmap.getAddr32(x,y) = SK_ColorWHITE;
                }
            }
        }
    }
    test->unlockPixels();
    baseline->unlockPixels();

    result->maxRedDiff = maxRedDiff;
    result->maxGreenDiff = maxGreenDiff;
    result->maxBlueDiff = maxBlueDiff;

    if (bitmapsToCreate.alphaMask) {
        result->poiAlphaMask.unlockPixels();
    }
    if (bitmapsToCreate.rgbDiff) {
        result->rgbDiffBitmap.unlockPixels();
    }
    if (bitmapsToCreate.whiteDiff) {
        result->whiteDiffBitmap.unlockPixels();
    }

    // Calculates the percentage of identical pixels
    result->result = 1.0 - ((double)result->poiCount / (width * height));
    result->timeElapsed = get_seconds() - startTime;

    return true;
}
