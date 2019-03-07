// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
REG_FIDDLE(Image_024, 256, 256, false, 5) {
// HASH=10172fca71b9dbdcade772513ffeb27e
void draw(SkCanvas* canvas) {
    SkMatrix matrix;
    matrix.setRotate(45);
    matrix.postTranslate(125, 30);
    SkPaint paint;
    paint.setShader(image->makeShader(&matrix));
    canvas->drawPaint(paint);
}
}
