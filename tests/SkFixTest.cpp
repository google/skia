/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkFix.h"

DEF_TEST(SkFix, r) {
    // For all v, v*0 == 0, v*1 == v.
    for (uint16_t bits = 0; bits <= 32768; bits++) {
        auto v = SkFix::Load(bits);
        REPORTER_ASSERT(r, v * 0.0f == 0.0f);
        REPORTER_ASSERT(r, v * 1.0f == v);
    }

    // Division and multiplication by powers of 2 is exact.
    SkFix v = 1.0f;
    REPORTER_ASSERT(r, (v>>1)    == 0.50f);
    REPORTER_ASSERT(r, (v>>2)    == 0.25f);
    REPORTER_ASSERT(r, (v>>2<<1) == 0.50f);

    // FromU8() should be just as good as going through float.
    for (int x = 0; x < 256; x++) {
        REPORTER_ASSERT(r, SkFix::FromU8(x) == SkFix(x * (1/255.0f)));
    }
}
