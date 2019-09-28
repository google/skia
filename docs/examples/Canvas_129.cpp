#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=00b430bd80d740e19c6d020a940f56d5
REG_FIDDLE(Canvas_129, 256, 1, false, 0) {
void draw(SkCanvas* canvas) {
    const char text[] = "Click this link!";
    SkRect bounds;
    SkPaint paint;
    paint.setTextSize(40);
    (void)paint.measureText(text, strlen(text), &bounds);
    const char url[] = "https://www.google.com/";
    sk_sp<SkData> urlData(SkData::MakeWithCString(url));
    canvas->drawAnnotation(bounds, "url_key", urlData.get());
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
