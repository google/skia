/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"

#include <initializer_list>

DEF_SIMPLE_GM(drrect_small_inner, canvas, 170, 610) {
    SkPaint paint;
    paint.setAntiAlias(true);
    static constexpr SkScalar kOuterRadius = 35.f;
    auto outer = SkRRect::MakeOval(SkRect::MakeXYWH(0, 0, 2 * kOuterRadius, 2 * kOuterRadius));
    canvas->translate(10.f, 10.f);
    canvas->save();
    for (bool offcenter : {false, true}) {
        for (bool oval : {false, true}) {
            for (SkScalar innerRadiusX : {1.f, 0.5f, 0.1f, .01f}) {
                SkScalar innerRadiusY = innerRadiusX;
                if (oval) {
                    innerRadiusY *= 0.95f;
                }
                SkScalar tx = kOuterRadius - innerRadiusX;
                SkScalar ty = kOuterRadius - innerRadiusY;
                if (offcenter) {
                    tx += 1.f;
                }
                auto inner = SkRRect::MakeOval(
                        SkRect::MakeXYWH(tx, ty, 2 * innerRadiusX, 2 * innerRadiusY));
                canvas->drawDRRect(outer, inner, paint);
                canvas->translate(0, 2 * kOuterRadius + 5);
            }
        }
        canvas->restore();
        canvas->translate(2 * kOuterRadius + 2, 0);
    }
    canvas->restore();
}
