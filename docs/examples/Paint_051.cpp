// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
REG_FIDDLE(Paint_051, 256, 256, true, 0) {
// HASH=50da74a43b725f07a914df588c867d36
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkDebugf("default miter limit == %g\n", paint.getStrokeMiter());
}
}
