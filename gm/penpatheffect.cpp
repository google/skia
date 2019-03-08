// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "gm.h"
#include "SkPenPathEffect.h"

static sk_sp<SkPathEffect> rot_pen(float x, float y, float angle) {
    SkMatrix m = SkMatrix::MakeScale(x, y);
    m.postRotate(angle);
    return SkMakePenPathEffect(m);
}
DEF_SIMPLE_GM(penpatheffect, canvas, 256, 256) {
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeCap(SkPaint::kRound_Cap);
    paint.setPathEffect(rot_pen(15, 5, -30));
    canvas->drawCircle(128, 128, 60, paint);
    SkPath path;
    path.moveTo(8, 8);
    path.quadTo(368, 68, 128, 128);
    path.quadTo(-112, 188, 248, 248);
    canvas->drawPath(path, paint);
}
