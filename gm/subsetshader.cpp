/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "SkBitmap.h"
#include "SkShader.h"
#include "gm.h"

DEF_SIMPLE_GM(bitmap_subset_shader, canvas, 256, 256) {
    canvas->clear(SK_ColorWHITE);

    SkBitmap source;
    if (!GetResourceAsBitmap("images/color_wheel.png", &source)) {
        return;
    }
    SkIRect left = SkIRect::MakeWH(source.width()/2, source.height());
    SkIRect right = SkIRect::MakeXYWH(source.width()/2, 0,
                                      source.width()/2, source.height());
    SkBitmap leftBitmap, rightBitmap;
    source.extractSubset(&leftBitmap, left);
    source.extractSubset(&rightBitmap, right);

    SkMatrix matrix;
    matrix.setScale(0.75f, 0.75f);
    matrix.preRotate(30.0f);
    SkShader::TileMode tm = SkShader::kRepeat_TileMode;
    SkPaint paint;
    paint.setShader(SkShader::MakeBitmapShader(leftBitmap, tm, tm, &matrix));
    canvas->drawRect(SkRect::MakeWH(256.0f, 128.0f), paint);
    paint.setShader(SkShader::MakeBitmapShader(rightBitmap, tm, tm, &matrix));
    canvas->drawRect(SkRect::MakeXYWH(0, 128.0f, 256.0f, 128.0f), paint);
}
