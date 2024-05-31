// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Matrix_MakeTrans, 256, 256, false, 4) {
void draw(SkCanvas* canvas) {
    SkMatrix matrix = SkMatrix::Translate(64, 48);
    for (int i = 0; i < 4; ++i) {
        canvas->drawImage(image, 0, 0);
        canvas->concat(matrix);
    }
}
}  // END FIDDLE
