// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=830a9e4e4bb93d25afd83b2fea63929e
REG_FIDDLE(Matrix_setPerspX, 256, 256, false, 4) {
void draw(SkCanvas* canvas) {
    for (SkScalar perspX : { -.003f, 0.f, .003f, .012f } ) {
        SkMatrix matrix;
        matrix.setIdentity();
        matrix.setPerspX(perspX);
        canvas->save();
        canvas->concat(matrix);
        canvas->drawBitmap(source, 0, 0);
        canvas->restore();
        canvas->translate(64, 64);
    }
}
}  // END FIDDLE
