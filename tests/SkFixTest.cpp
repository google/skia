/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkFix.h"

DEF_TEST(SkFix, r) {
    for (uint16_t bits = 0; bits <= 32768; bits++) {
        auto v = SkFix::Load(bits);
        REPORTER_ASSERT(r, v * 0.0f == 0.0f);
        REPORTER_ASSERT(r, v * 1.0f == v);
    }

    SkFix v = 1.0f;
    REPORTER_ASSERT(r, (v>>1)    == 0.50f);
    REPORTER_ASSERT(r, (v>>2)    == 0.25f);
    REPORTER_ASSERT(r, (v>>2<<1) == 0.50f);
}
