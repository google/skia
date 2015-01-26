/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Checkerboard.h"
#include "SkCanvas.h"
#include "SkShader.h"

SkShader* sk_tools::CreateCheckerboardShader(
        SkColor c1, SkColor c2, int size) {
    SkBitmap bm;
    bm.allocN32Pixels(2 * size, 2 * size);
    bm.eraseColor(c1);
    bm.eraseArea(SkIRect::MakeLTRB(0, 0, size, size), c2);
    bm.eraseArea(SkIRect::MakeLTRB(size, size, 2 * size, 2 * size), c2);
    return SkShader::CreateBitmapShader(
            bm, SkShader::kRepeat_TileMode, SkShader::kRepeat_TileMode);
}

void sk_tools::DrawCheckerboard(SkCanvas* canvas,
                                    SkColor c1, SkColor c2, int size) {
    SkPaint paint;
    paint.setShader(CreateCheckerboardShader(c1, c2, size))->unref();
    canvas->drawPaint(paint);
}
