// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=658c1df611b4577cc7e0bb384e95737e
REG_FIDDLE(IPoint_003, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkIPoint pt = { 0, -0};
    SkDebugf("pt.isZero() == %s\n", pt.isZero() ? "true" : "false");
}
}  // END FIDDLE
