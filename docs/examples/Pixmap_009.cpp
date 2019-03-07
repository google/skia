// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
REG_FIDDLE(Pixmap_009, 256, 256, true, 0) {
// HASH=f68617b7153a20b2ed3d7f9ed5c6e5e4
void draw(SkCanvas* canvas) {
    SkImageInfo info = SkImageInfo::MakeA8(16, 32);
    SkPixmap pixmap(info, nullptr, 64);
    SkDebugf("pixmap width: %d  info width: %d\n", pixmap.width(), info.width());
}
}
