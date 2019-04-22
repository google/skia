// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=30823cb4edf884d330285ea161664931
REG_FIDDLE(Canvas_drawDRRect_b, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
   SkRRect outer = SkRRect::MakeRect({20, 40, 210, 200});
   SkRRect inner = SkRRect::MakeRectXY({60, 70, 170, 160}, 10, 10);
   SkPaint paint;
   paint.setAntiAlias(true);
   paint.setStyle(SkPaint::kStroke_Style);
   paint.setStrokeWidth(20);
   paint.setStrokeJoin(SkPaint::kRound_Join);
   canvas->drawDRRect(outer, inner, paint);
   paint.setStrokeWidth(1);
   paint.setColor(SK_ColorWHITE);
   canvas->drawDRRect(outer, inner, paint);
}
}  // END FIDDLE
