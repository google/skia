// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=cfe4016241074477809dd45435be9cf4
REG_FIDDLE(Canvas_quickReject, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect testRect = {30, 30, 120, 129 };
    SkRect clipRect = {30, 130, 120, 230 };
    canvas->save();
    canvas->clipRect(clipRect);
    SkDebugf("quickReject %s\n", canvas->quickReject(testRect) ? "true" : "false");
    canvas->restore();
    canvas->rotate(10);
    canvas->clipRect(clipRect);
    SkDebugf("quickReject %s\n", canvas->quickReject(testRect) ? "true" : "false");
}
}  // END FIDDLE
