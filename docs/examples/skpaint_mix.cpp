// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(skpaint_mix, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint fillPaint;
    SkPaint strokePaint;
    strokePaint.setStyle(SkPaint::kStroke_Style);
    strokePaint.setStrokeWidth(3.0f);

    canvas->drawRect(SkRect::MakeXYWH(10, 10, 60, 20), fillPaint);
    canvas->drawRect(SkRect::MakeXYWH(80, 10, 60, 20), strokePaint);

    strokePaint.setStrokeWidth(5.0f);
    canvas->drawOval(SkRect::MakeXYWH(150, 10, 60, 20), strokePaint);

    sk_sp<SkTextBlob> blob = SkTextBlob::MakeFromString("SKIA", SkFont(nullptr, 80));

    fillPaint.setColor(SkColorSetARGB(0xFF, 0xFF, 0x00, 0x00));
    canvas->drawTextBlob(blob.get(), 20, 120, fillPaint);

    fillPaint.setColor(SkColorSetARGB(0xFF, 0x00, 0x00, 0xFF));
    canvas->drawTextBlob(blob.get(), 20, 220, fillPaint);
}
}  // END FIDDLE
