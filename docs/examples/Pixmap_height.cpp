// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Pixmap_height, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPixmap pixmap(SkImageInfo::MakeA8(16, 32), nullptr, 64);
    SkDebugf("pixmap height: %d  info height: %d\n", pixmap.height(), pixmap.info().height());
}
}  // END FIDDLE
