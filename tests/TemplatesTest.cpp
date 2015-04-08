/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTemplates.h"
#include "Test.h"

// Tests for some of the helpers in SkTemplates.h
static void test_automalloc_realloc(skiatest::Reporter* reporter) {
    SkAutoSTMalloc<1, int> array;

    // test we have a valid pointer, should not crash
    array[0] = 1;
    REPORTER_ASSERT(reporter, array[0] == 1);

    // using realloc for init
    array.realloc(1);

    array[0] = 1;
    REPORTER_ASSERT(reporter, array[0] == 1);

    // verify realloc can grow
    array.realloc(2);
    REPORTER_ASSERT(reporter, array[0] == 1);

    // realloc can shrink
    array.realloc(1);
    REPORTER_ASSERT(reporter, array[0] == 1);

    // should not crash
    array.realloc(0);

    // grow and shrink again
    array.realloc(10);
    for (int i = 0; i < 10; i++) {
        array[i] = 10 - i;
    }
    array.realloc(20);
    for (int i = 0; i < 10; i++) {
        REPORTER_ASSERT(reporter, array[i] == 10 - i);
    }
    array.realloc(10);
    for (int i = 0; i < 10; i++) {
        REPORTER_ASSERT(reporter, array[i] == 10 - i);
    }

    array.realloc(1);
    REPORTER_ASSERT(reporter, array[0] = 10);

    // resets mixed with realloc, below stack alloc size
    array.reset(0);
    array.realloc(1);
    array.reset(1);

    array[0] = 1;
    REPORTER_ASSERT(reporter, array[0] == 1);

    // reset and realloc > stack size
    array.reset(2);
    array.realloc(3);
    array[0] = 1;
    REPORTER_ASSERT(reporter, array[0] == 1);
    array.realloc(1);
    REPORTER_ASSERT(reporter, array[0] == 1);
}

DEF_TEST(Templates, reporter) {
    test_automalloc_realloc(reporter);
}
