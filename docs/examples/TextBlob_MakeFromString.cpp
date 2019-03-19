#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=a5af182e793eed3f2bb3b0efc2cf4852
REG_FIDDLE(TextBlob_MakeFromString, 256, 24, false, 0) {
void draw(SkCanvas* canvas) {
    SkFont font;
    font.setSize(24);
    SkPaint canvasPaint;
    canvasPaint.setColor(SK_ColorBLUE); // respected
    canvasPaint.setTextSize(2); // ignored
    sk_sp<SkTextBlob> blob = SkTextBlob::MakeFromString("Hello World", font);
    canvas->drawTextBlob(blob, 20, 20, canvasPaint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
