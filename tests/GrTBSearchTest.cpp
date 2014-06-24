/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test
#if SK_SUPPORT_GPU

#include "Test.h"

// If we aren't inheriting these as #defines from elsewhere,
// clang demands they be declared before we #include the template
// that relies on them.
static bool LT(const int& elem, int value) {
    return elem < value;
}
static bool EQ(const int& elem, int value) {
    return elem == value;
}

#include "GrTBSearch.h"

DEF_TEST(GrTBSearch, reporter) {
    const int array[] = {
        1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 22, 33, 44, 55, 66, 77, 88, 99
    };

    for (int n = 0; n < static_cast<int>(SK_ARRAY_COUNT(array)); ++n) {
        for (int i = 0; i < n; i++) {
            int index = GrTBSearch<int, int>(array, n, array[i]);
            REPORTER_ASSERT(reporter, index == (int) i);
            index = GrTBSearch<int, int>(array, n, -array[i]);
            REPORTER_ASSERT(reporter, index < 0);
        }
    }
}

#endif
