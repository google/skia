/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColor.h"
#include "tests/Test.h"

DEF_TEST(SkColor4f_FromColor, reporter) {
    const struct {
        SkColor     fC;
        SkColor4f   fC4;
    } recs[] = {
        { SK_ColorBLACK, { 0, 0, 0, 1 } },
        { SK_ColorWHITE, { 1, 1, 1, 1 } },
        { SK_ColorRED,   { 1, 0, 0, 1 } },
        { SK_ColorGREEN, { 0, 1, 0, 1 } },
        { SK_ColorBLUE,  { 0, 0, 1, 1 } },
        { 0,             { 0, 0, 0, 0 } },
    };

    for (const auto& r : recs) {
        SkColor4f c4 = SkColor4f::FromColor(r.fC);
        REPORTER_ASSERT(reporter, c4 == r.fC4);
    }
}
