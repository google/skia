/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpaceXform_v2.h"
#include "Test.h"
#include <stdlib.h>

static inline void assert_eq(skiatest::Reporter* r, float x, float y, int tol = 1) {
    int32_t X,Y;
    memcpy(&X, &x, 4);
    memcpy(&Y, &y, 4);

    if ((X >> 31) != (Y >> 31)) {
        ERRORF(r, "expected %g (0x%08x) == %g (0x%08x), but they have different signs\n", x,X, y,Y);
        return;
    }

    if (abs(X - Y) > tol) {
        ERRORF(r, "expected %g (0x%08x) and %g (0x%08x) to be within %d ulps\n", x,X, y,Y, tol);
    }
}

DEF_TEST(ColorSpaceXform_v2, r) {

    assert_eq(r, 3.101f, 3.1);

}
