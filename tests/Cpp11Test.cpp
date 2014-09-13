/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

DEF_TEST(Cpp11, r) {
    auto square = [](int x){ return x*x; };

    REPORTER_ASSERT(r, square(4) == 16);
}
