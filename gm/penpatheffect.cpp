// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "gm.h"
#include "SkPenPathEffect.h"
static sk_sp<SkPathEffect> rot_pen(float x, float y, float angle) {
    SkMatrix m = SkMatrix::MakeScale(x, y);
    m.postRotate(angle);
    return SkMakePenPathEffect(m[0], m[1], m[3], m[4]);
}
DEF_SIMPLE_GM(penpatheffect, canvas, 256, 256) {
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setPathEffect(rot_pen(15, 5, -30));
    canvas->drawCircle(128, 128, 64, paint);
    SkPath path;
    path.moveTo(10, 10);
    path.quadTo(384, 64, 128, 128);
    path.quadTo(-128, 192, 246, 246);
    canvas->drawPath(path, paint);

}
