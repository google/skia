/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkScaleToSides.h"

#include <algorithm>
#include "Test.h"

DEF_TEST(ScaleToSides, reporter) {
    double interestingValues[] = {
        // From skp bitbucket
        111.60000228881836,
        55.800003051757813,
        0.99999996581812677920,
        0.0,
        0.5,
        1.0,
        2.0,
        3.0,
        33.0,
        33554430.0,
        33554431.0,
        33554464.0,
        333333332.0,
        333333333.0,
        333333334.0,
        FLT_MAX,
        FLT_EPSILON,
        FLT_MIN
    };

    int numInterestingValues = (int)SK_ARRAY_COUNT(interestingValues);

    for (int s = 0; s <= numInterestingValues; s++) {
        for (int i = 0; i < numInterestingValues; i++) {
            for (int j = 0; j < numInterestingValues; j++) {
                for (int k = 0; k < numInterestingValues; k++) {
                    float radius1 = (float)interestingValues[i];
                    float radius2 = (float)interestingValues[j];
                    double width = interestingValues[k];
                    double scale = width / ((double)radius1 + (double)radius2);
                    if (width > 0.0) {
                        if (s != 0) {
                            scale = std::min(scale, interestingValues[s-1]);
                        }
                        if (scale < 1.0 && scale > 0.0) {
                            SkScaleToSides::AdjustRadii(width, scale, &radius1, &radius2);
                        }
                    }
                }
            }
        }
    }
}
