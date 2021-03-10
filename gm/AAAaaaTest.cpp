/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/effects/SkColorMatrix.h"
#include "include/effects/SkGradientShader.h"

DEF_SIMPLE_GM(AAAaaaTest, canvas, 512, 512) {
    int width = 500;
    int height = 500;
    SkPaint paint;
    // Simple scale/translate case where R, G, and B are all treated equivalently
    SkColorMatrix cm;
    cm.setScale(1.1f, 1.1f, 1.1f, 0.5f);
    cm.postTranslate(5.0f/255, 5.0f/255, 5.0f/255, 10.0f/255);

    paint.setColorFilter(SkColorFilters::Matrix(cm));

    // set a shader so it's not likely for the matrix to be optimized away (since a
    // clever
    // enough renderer might apply it directly to the paint color)
#if 0
    float pos[] = {0, 1};
    SkPoint pts[] = {SkPoint::Make(0, 0), SkPoint::Make(width, height)};
    SkColor colors[2] = {SK_ColorMAGENTA, SK_ColorYELLOW};
    paint.setShader(SkGradientShader::MakeLinear(pts, colors, pos, 2,
                                                 SkTileMode::kClamp));
#endif

    // overdraw several times to emphasize shader cost
    for (int i = 0; i < 100; i++) {
        canvas->drawRect(SkRect::MakeLTRB(i, i, width, height), paint);
    }
}
