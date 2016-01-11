/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkScaleToSides.h"

#include <cfloat>
#include "Test.h"

DEF_TEST(ScaleToSides, reporter) {
    float interestingValues[] = {
        0.0f,
        0.5f,
        1.0f,
        2.0f,
        3.0f,
        33.0f,
        33554430.0f,
        33554431.0f,
        33554464.0f,
        333333332.0f,
        333333333.0f,
        333333334.0f,
        FLT_MAX,
        FLT_EPSILON,
        FLT_MIN
    };

    int numInterestingValues = (int)SK_ARRAY_COUNT(interestingValues);

    for (int i = 0; i < numInterestingValues; i++) {
        for (int j = 0; j < numInterestingValues; j++) {
            for (int k = 0; k < numInterestingValues; k++) {
                float radius1 = interestingValues[i];
                float radius2 = interestingValues[j];
                float width = interestingValues[k];
                if (width > 0.0f) {
                    double scale = (double)width / ((double)radius1 + (double)radius2);
                    if (scale < 1.0) {
                        ScaleToSides::AdjustRadii(width, scale, &radius1, &radius2);
                    }
                }
            }
        }
    }
}
