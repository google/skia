#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=bf0b2402533a23b6392e0676b7a8414c
REG_FIDDLE(Canvas_drawPosText, 256, 120, false, 0) {
void draw(SkCanvas* canvas) {
  const char hello[] = "HeLLo!";
  const SkPoint pos[] = { {40, 100}, {82, 95}, {115, 110}, {130, 95}, {145, 85},
    {172, 100} };
  SkPaint paint;
  paint.setTextSize(60);
  canvas->drawPosText(hello, strlen(hello), pos, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
