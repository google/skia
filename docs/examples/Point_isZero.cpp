// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=81b9665110b88ef6bcbc20464aed7da1
REG_FIDDLE(Point_isZero, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPoint pt = { 0.f, -0.f};
    SkDebugf("pt.fX=%c%g pt.fY=%c%g\n", std::signbit(pt.fX) ? '-' : '+', fabsf(pt.fX),
                                        std::signbit(pt.fY) ? '-' : '+', fabsf(pt.fY));
    SkDebugf("pt.isZero() == %s\n", pt.isZero() ? "true" : "false");
}
}  // END FIDDLE
