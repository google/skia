// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE_SRGB(bug7573_1, 260, 260, false, 5, 0, false) {
void draw(SkCanvas* canvas) {
    SkPaint p;
    p.setAntiAlias(true);
    SkPath path;
path.moveTo(1.98009784, 9.0162744);
path.lineTo(47.843992, 10.1922744);
path.lineTo(47.804008, 11.7597256);
path.lineTo(1.93990216, 10.5837256);
    canvas->drawPath(path, p);
}
}  // END FIDDLE
