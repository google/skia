// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(pathmeasure, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.moveTo(0, 1);
    path.lineTo(1, 1);
    SkPathMeasure measure(path, false);
    SkPath result;
    measure.getSegment(.5, 1.5, &result, true);
    result.dump();
}
}  // END FIDDLE
