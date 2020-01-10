// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(makeRasterImage_fail, 256, 256, false, 5) {
void draw(SkCanvas* canvas) { canvas->drawImage(image->makeRasterImage(), 0, 0); }
}  // END FIDDLE
