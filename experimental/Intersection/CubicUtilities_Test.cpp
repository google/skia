/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Intersection_Tests.h"
#include "CubicUtilities.h"

const Cubic tests[] = {
    {{2, 0}, {3, 1}, {2, 2}, {1, 1}},
    {{3, 1}, {2, 2}, {1, 1}, {2, 0}},
    {{3, 0}, {2, 1}, {3, 2}, {1, 1}},
};

const size_t tests_count = sizeof(tests) / sizeof(tests[0]);
static size_t firstLineParameterTest = 0;

void CubicUtilities_Test() {
    for (size_t index = firstLineParameterTest; index < tests_count; ++index) {
        const Cubic& cubic = tests[index];
        bool result = clockwise(cubic);
        if (!result) {
            SkDebugf("%s [%d] expected clockwise\n", __FUNCTION__, index);
            SkASSERT(0);
        }
    }
}
