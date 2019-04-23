/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkColorData.h"
#include "tests/Test.h"

// our std SkAlpha255To256
static int test_srcover0(unsigned dst, unsigned alpha) {
    return alpha + SkAlphaMul(dst, SkAlpha255To256(255 - alpha));
}

// faster hack +1
static int test_srcover1(unsigned dst, unsigned alpha) {
    return alpha + SkAlphaMul(dst, 256 - alpha);
}

// slower "correct"
static int test_srcover2(unsigned dst, unsigned alpha) {
    return alpha + SkMulDiv255Round(dst, 255 - alpha);
}

DEF_TEST(SrcOver, reporter) {
    /*  Here's the idea. Can we ensure that when we blend on top of an opaque
        dst, that the result always stay's opaque (i.e. exactly 255)?
     */

    unsigned i;
    int opaqueCounter0 = 0;
    int opaqueCounter1 = 0;
    int opaqueCounter2 = 0;
    for (i = 0; i <= 255; i++) {
        unsigned result0 = test_srcover0(0xFF, i);
        unsigned result1 = test_srcover1(0xFF, i);
        unsigned result2 = test_srcover2(0xFF, i);
        opaqueCounter0 += (result0 == 0xFF);
        opaqueCounter1 += (result1 == 0xFF);
        opaqueCounter2 += (result2 == 0xFF);
    }
#if 0
    INFOF(reporter, "---- opaque test: [%d %d %d]\n",
          opaqueCounter0, opaqueCounter1, opaqueCounter2);
#endif
    // we acknowledge that technique0 does not always return opaque
    REPORTER_ASSERT(reporter, opaqueCounter0 == 256);
    REPORTER_ASSERT(reporter, opaqueCounter1 == 256);
    REPORTER_ASSERT(reporter, opaqueCounter2 == 256);

    // Now ensure that we never over/underflow a byte
    for (i = 0; i <= 255; i++) {
        for (unsigned dst = 0; dst <= 255; dst++) {
            unsigned r0 = test_srcover0(dst, i);
            unsigned r1 = test_srcover1(dst, i);
            unsigned r2 = test_srcover2(dst, i);
            unsigned max = SkMax32(dst, i);
            // ignore the known failure
            if (dst != 255) {
                REPORTER_ASSERT(reporter, r0 <= 255 && r0 >= max);
            }
            REPORTER_ASSERT(reporter, r1 <= 255 && r1 >= max);
            REPORTER_ASSERT(reporter, r2 <= 255 && r2 >= max);

#if 0
            // this shows where r1 (faster) differs from r2 (more exact)
            if (r1 != r2) {
                INFOF(reporter, "--- dst=%d i=%d r1=%d r2=%d exact=%g\n",
                      dst, i, r1, r2, i + dst - dst*i/255.0f);
            }
#endif
        }
    }
}
