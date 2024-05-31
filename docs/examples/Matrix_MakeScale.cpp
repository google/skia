// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Matrix_MakeScale, 256, 256, false, 4) {
void draw(SkCanvas* canvas) {
    canvas->concat(SkMatrix::Scale(4, 3));
    canvas->drawImage(image, 0, 0);
}
}  // END FIDDLE
