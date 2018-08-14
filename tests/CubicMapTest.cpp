/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCubicMap.h"
#include "SkRandom.h"
#include "Test.h"

static bool nearly_le(SkScalar a, SkScalar b) {
    return a <= b || SkScalarNearlyZero(a - b);
}

static void exercise_cubicmap(const SkCubicMap& cmap, skiatest::Reporter* reporter) {
    SkScalar prev_y = 0;
    for (SkScalar x = 0; x <= 1; x += 1.0f / 512) {
        SkScalar y = cmap.computeYFromX(x);
        if (y < 0 || y > 1 || !nearly_le(prev_y, y)) {
            cmap.computeYFromX(x);
            REPORTER_ASSERT(reporter, false);
        }
        prev_y = y;
    }
}

DEF_TEST(CubicMap, r) {
    const SkScalar values[] = {
        0, 1, 0.25f, 0.75f, 0.5f, 1.0f/3, 2.0f/3, 0.0000001f, 0.999999f,
    };

    for (SkScalar x0 : values) {
        for (SkScalar y0 : values) {
            for (SkScalar x1 : values) {
                for (SkScalar y1 : values) {
                    exercise_cubicmap(SkCubicMap({ x0, y0 }, { x1, y1 }), r);
                }
            }
        }
    }
}
