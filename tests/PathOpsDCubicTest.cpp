/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsTestCommon.h"
#include "SkPathOpsCubic.h"
#include "Test.h"

static const SkDCubic tests[] = {
    {{{2, 0}, {3, 1}, {2, 2}, {1, 1}}},
    {{{3, 1}, {2, 2}, {1, 1}, {2, 0}}},
    {{{3, 0}, {2, 1}, {3, 2}, {1, 1}}},
};

static const size_t tests_count = SK_ARRAY_COUNT(tests);

DEF_TEST(PathOpsDCubic, reporter) {
    for (size_t index = 0; index < tests_count; ++index) {
        const SkDCubic& cubic = tests[index];
        SkASSERT(ValidCubic(cubic));
        bool result = cubic.clockwise();
        if (!result) {
            SkDebugf("%s [%d] expected clockwise\n", __FUNCTION__, index);
            REPORTER_ASSERT(reporter, 0);
        }
    }
}
