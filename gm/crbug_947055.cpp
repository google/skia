/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"

DEF_SIMPLE_GM_BG(crbug_947055, canvas, 200, 50, SK_ColorBLUE) {
    // Green 2D rectangle to highlight the red rectangle. Isn't necessary
    // to trigger problem, but helps show the extreme corner outsets.
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorGREEN);
    canvas->drawRect(SkRect::MakeXYWH(19.f, 7.f, 180.f, 10.f), paint);

    // Red perspective rectangle with bad AA on Ganesh
    paint.setColor(SK_ColorRED);
    canvas->concat(SkMatrix::MakeAll(1.0f,  2.4520f, 19.0f,
                                     0.0f,  0.3528f,  9.5f,
                                     0.0f,  0.0225f,  1.0f));
    canvas->drawRect(SkRect::MakeWH(180.f, 500.f), paint);
}
