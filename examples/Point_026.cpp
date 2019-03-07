// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
namespace {
REG_FIDDLE(Point_026, 256, 256, true, 0);
// HASH=4cecb878c8b66beffda051f26c00f817
void draw(SkCanvas* canvas) {
    SkPoint test[] = { {0, -0.f}, {-1, -2}, {SK_ScalarInfinity, 1}, {SK_ScalarNaN, -1} };
    for (const SkPoint& pt : test) {
        SkDebugf("pt: %g, %g  %c= pt\n", pt.fX, pt.fY, pt.equals(pt.fX, pt.fY) ? '=' : '!');
    }
}
}
