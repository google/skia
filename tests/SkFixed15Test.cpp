/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkFixed15.h"

DEF_TEST(SkFixed15, r) {
    // For all v, v*0 == 0, v*1 == v.
    for (uint16_t bits = 0; bits <= 32768; bits++) {
        auto v = SkFixed15::Load(bits);
        REPORTER_ASSERT(r, v * 0.0f == 0.0f);
        REPORTER_ASSERT(r, v * 1.0f == v);
    }

    // Division and multiplication by powers of 2 is exact.
    SkFixed15 v = 1.0f;
    REPORTER_ASSERT(r, (v>>1)    == 0.50f);
    REPORTER_ASSERT(r, (v>>2)    == 0.25f);
    REPORTER_ASSERT(r, (v>>2<<1) == 0.50f);

    // FromU8() should be just as good as going through float.
    for (int x = 0; x < 256; x++) {
        REPORTER_ASSERT(r, SkFixed15::FromU8(x) == SkFixed15(x * (1/255.0f)));
    }

    // to_u8() and FromU8() should roundtrip all bytes.
    for (int x = 0; x < 256; x++) {
        REPORTER_ASSERT(r, x == SkFixed15::FromU8(x).to_u8());
    }
}
