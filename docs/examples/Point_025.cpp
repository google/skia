// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
REG_FIDDLE(Point_025, 256, 256, true, 0) {
// HASH=937cc166cc0e220f33fb82501141d0b3
void draw(SkCanvas* canvas) {
    SkPoint test[] = { {0, -0.f}, {-1, -2}, {SK_ScalarInfinity, 1}, {SK_ScalarNaN, -1} };
    for (const SkPoint& pt : test) {
        SkDebugf("pt: %g, %g  finite: %s\n", pt.fX, pt.fY, pt.isFinite() ? "true" : "false");
    }
}
}
