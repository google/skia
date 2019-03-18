// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=10172fca71b9dbdcade772513ffeb27e
REG_FIDDLE(Image_makeShader_2, 256, 256, false, 5) {
void draw(SkCanvas* canvas) {
    SkMatrix matrix;
    matrix.setRotate(45);
    matrix.postTranslate(125, 30);
    SkPaint paint;
    paint.setShader(image->makeShader(&matrix));
    canvas->drawPaint(paint);
}
}  // END FIDDLE
