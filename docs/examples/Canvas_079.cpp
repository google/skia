// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=02e33141f13da2f19aef7feb7117b541
REG_FIDDLE(Canvas_079, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
   SkRRect outer = SkRRect::MakeRect({20, 40, 210, 200});
   SkRRect inner = SkRRect::MakeOval({60, 70, 170, 160});
   SkPaint paint;
   canvas->drawDRRect(outer, inner, paint);
}
}  // END FIDDLE
