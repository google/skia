// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=13cf9e7b2894ae6e98c1fd719040bf01
REG_FIDDLE(Surface_026, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    const char* names[] = { "Unknown", "RGB_H", "BGR_H", "RGB_V", "BGR_V" };
    sk_sp<SkSurface> surf(SkSurface::MakeRasterN32Premul(64, 64));
    SkDebugf("surf.props(): k%s_SkPixelGeometry\n", names[surf->props().pixelGeometry()]);
}
}  // END FIDDLE
