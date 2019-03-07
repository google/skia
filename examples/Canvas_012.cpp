// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
namespace {
REG_FIDDLE(Canvas_012, 256, 256, false, 0);
// HASH=c4ea949e5fa5a0630dcb6b0204bd498f
void draw(SkCanvas* canvas) {
    if (canvas->getGrContext()) {
         canvas->clear(SK_ColorRED);
    } else {
         canvas->clear(SK_ColorBLUE);
    }
}

}
