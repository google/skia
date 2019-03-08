// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "gm.h"
#include "SkPenPathEffect.h"
DEF_SIMPLE_GM(penpatheffect, canvas, 256, 256) {
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(10);
    SkMatrix m = SkMatrix::MakeScale(4, 1);
    m.postRotate(45);
    paint.setPathEffect(SkMakePenPathEffect(m[0], m[1], m[3], m[4]));
    canvas->drawCircle(128, 128, 100, paint);
}
