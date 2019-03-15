// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=12b7164a769e232bb772f19c59600ee7
REG_FIDDLE(Point_006, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkIPoint iPt = { SK_MinS32, SK_MaxS32 };
    SkPoint fPt;
    fPt.iset(iPt);
    SkDebugf("iPt: %d, %d\n", iPt.fX, iPt.fY);
    SkDebugf("fPt: %g, %g\n", fPt.fX, fPt.fY);
}
}  // END FIDDLE
