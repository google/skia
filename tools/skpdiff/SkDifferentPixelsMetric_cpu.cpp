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

bool SkDifferentPixelsMetric::diff(SkBitmap* baseline, SkBitmap* test, bool computeMask,
                                   Result* result) const {
    double startTime = get_seconds();

    // Ensure the images are comparable
    if (baseline->width() != test->width() || baseline->height() != test->height() ||
        baseline->width() <= 0 || baseline->height() <= 0 ||
        baseline->config() != test->config()) {
        return false;
    }

    int width = baseline->width();
    int height = baseline->height();

    // Prepare the POI alpha mask if needed
    if (computeMask) {
        result->poiAlphaMask.setConfig(SkBitmap::kA8_Config, width, height);
        result->poiAlphaMask.allocPixels();
        result->poiAlphaMask.lockPixels();
        result->poiAlphaMask.eraseARGB(SK_AlphaOPAQUE, 0, 0, 0);
    }

    // Prepare the pixels for comparison
    result->poiCount = 0;
    baseline->lockPixels();
    test->lockPixels();
    for (int y = 0; y < height; y++) {
        // Grab a row from each image for easy comparison
        unsigned char* baselineRow = (unsigned char*)baseline->getAddr(0, y);
        unsigned char* testRow = (unsigned char*)test->getAddr(0, y);
        for (int x = 0; x < width; x++) {
            // Compare one pixel at a time so each differing pixel can be noted
            if (memcmp(&baselineRow[x * 4], &testRow[x * 4], 4) != 0) {
                result->poiCount++;
                if (computeMask) {
                    *result->poiAlphaMask.getAddr8(x,y) = SK_AlphaTRANSPARENT;
                }
            }
        }
    }
    test->unlockPixels();
    baseline->unlockPixels();

    if (computeMask) {
        result->poiAlphaMask.unlockPixels();
    }

    // Calculates the percentage of identical pixels
    result->result = 1.0 - ((double)result->poiCount / (width * height));
    result->timeElapsed = get_seconds() - startTime;

    return true;
}
