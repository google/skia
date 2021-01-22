/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkScaleToSides.h"

#include "tests/Test.h"

#include <algorithm>

DEF_TEST(ScaleToSides, reporter) {
    double interestingValues[] = {
        // From sample app - PathFuzzer
        260.01662826538085938,
        63.61007690429687500,
        795.98901367187500000,
        217.71697616577148438,
        686.15960693359375000,
        556.57641601562500000,
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
        FLT_MIN,
        340282569745034499980078846904281071616.0,
        170141284872517249990039423452140535808.0,
        170141244307698042686698575557637963776.0,
    };

    int numInterestingValues = (int)SK_ARRAY_COUNT(interestingValues);

    for (int s = 0; s <= numInterestingValues; s++) {
        for (int i = 0; i < numInterestingValues; i++) {
            for (int j = 0; j < numInterestingValues; j++) {
                for (int k = 0; k < numInterestingValues; k++) {
                    // We're about to cast values i and j to float, don't bother if they won't fit.
                    // (Is there a more robust way to test this, like SkTFitsIn but double->float?)
                    if (interestingValues[i] > FLT_MAX ||
                        interestingValues[j] > FLT_MAX) {
                        continue;
                    }
                    float radius1 = (float)interestingValues[i];
                    float radius2 = (float)interestingValues[j];
                    double width = interestingValues[k];
                    double scale = sk_ieee_double_divide(width, (double)radius1 + (double)radius2);
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
