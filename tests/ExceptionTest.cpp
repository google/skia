/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

// Just a little meta-test that our test tools can compile and link with exceptions enabled.
DEF_TEST(Exceptions, r) {
    bool exception_caught = false;
    try {
        throw 42;
    } catch (...) {
        exception_caught = true;
    }
    REPORTER_ASSERT(r, exception_caught);
}
