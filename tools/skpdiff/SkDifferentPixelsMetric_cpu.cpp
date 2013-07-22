/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <cstring>

#include "SkBitmap.h"

#include "SkDifferentPixelsMetric.h"
#include "skpdiff_util.h"

struct SkDifferentPixelsMetric::QueuedDiff {
    bool finished;
    double result;
    SkTDArray<SkIPoint>* poi;
};

const char* SkDifferentPixelsMetric::getName() {
    return "different_pixels";
}

int SkDifferentPixelsMetric::queueDiff(SkBitmap* baseline, SkBitmap* test) {
    double startTime = get_seconds();
    int diffID = fQueuedDiffs.count();
    QueuedDiff* diff = fQueuedDiffs.push();
    SkTDArray<SkIPoint>* poi = diff->poi = new SkTDArray<SkIPoint>();

    // If we never end up running the kernel, include some safe defaults in the result.
    diff->finished = false;
    diff->result = -1;

    // Ensure the images are comparable
    if (baseline->width() != test->width() || baseline->height() != test->height() ||
        baseline->width() <= 0 || baseline->height() <= 0 ||
        baseline->config() != test->config()) {
        diff->finished = true;
        return diffID;
    }

    int width = baseline->width();
    int height = baseline->height();
    int differentPixelsCount = 0;

    // Prepare the pixels for comparison
    baseline->lockPixels();
    test->lockPixels();
    for (int y = 0; y < height; y++) {
        // Grab a row from each image for easy comparison
        unsigned char* baselineRow = (unsigned char*)baseline->getAddr(0, y);
        unsigned char* testRow = (unsigned char*)test->getAddr(0, y);
        for (int x = 0; x < width; x++) {
            // Compare one pixel at a time so each differing pixel can be noted
            if (std::memcmp(&baselineRow[x * 4], &testRow[x * 4], 4) != 0) {
                poi->push()->set(x, y);
                differentPixelsCount++;
            }
        }
    }
    test->unlockPixels();
    baseline->unlockPixels();

    // Calculates the percentage of identical pixels
    diff->result = 1.0 - ((double)differentPixelsCount / (width * height));

    SkDebugf("Time: %f\n", (get_seconds() - startTime));

    return diffID;
}

void SkDifferentPixelsMetric::deleteDiff(int id) {
    if (NULL != fQueuedDiffs[id].poi)
    {
        delete fQueuedDiffs[id].poi;
        fQueuedDiffs[id].poi = NULL;
    }
}

bool SkDifferentPixelsMetric::isFinished(int id) {
    return fQueuedDiffs[id].finished;
}

double SkDifferentPixelsMetric::getResult(int id) {
    return fQueuedDiffs[id].result;
}

int SkDifferentPixelsMetric::getPointsOfInterestCount(int id) {
    return fQueuedDiffs[id].poi->count();
}

SkIPoint* SkDifferentPixelsMetric::getPointsOfInterest(int id) {
    return fQueuedDiffs[id].poi->begin();
}
