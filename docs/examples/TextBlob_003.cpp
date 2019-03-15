#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=bec2252bc36dc8fd023015629d60c405
REG_FIDDLE(TextBlob_003, 256, 24, false, 0) {
void draw(SkCanvas* canvas) {
    SkFont font;
    font.setSize(24);
    SkPaint canvasPaint;
    canvasPaint.setColor(SK_ColorBLUE); // respected
    canvasPaint.setTextSize(2); // ignored
    sk_sp<SkTextBlob> blob = SkTextBlob::MakeFromText("Hello World", 11, font);
    canvas->drawTextBlob(blob, 20, 20, canvasPaint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
