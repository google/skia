/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/effects/SkDashPathEffect.h"

DEF_SIMPLE_GM(crbug_1113794, canvas, 600, 200) {
    SkPath path = SkPath::Line({50.f, 80.f}, {50.f, 20.f});

    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    paint.setAntiAlias(true);
    paint.setStrokeWidth(0.25f);
    paint.setStyle(SkPaint::kStroke_Style);

    static constexpr SkScalar kDash[2] = {10.f, 10.f};
    paint.setPathEffect(SkDashPathEffect::Make(kDash, 2, 0.f));

    SkMatrix viewBox = SkMatrix::RectToRect(SkRect::MakeWH(100, 100), SkRect::MakeWH(600, 200));
    canvas->concat(viewBox);

    canvas->drawPath(path, paint);
}
