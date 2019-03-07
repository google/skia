// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
namespace {
REG_FIDDLE(Canvas_024, 256, 128, false, 0);
// HASH=9f563a2d60aa31d4b26742e5aa17aa4e
void draw(SkCanvas* canvas) {
    canvas->clipRect(SkRect::MakeWH(100, 100));
    canvas->clear(SK_ColorRED);
    canvas->scale(.5, .5);
    canvas->clipRect(SkRect::MakeWH(100, 100));
    canvas->clear(SK_ColorBLUE);
}

}
