// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=1ce28351444b41ab2b8e3128a4b9b9c2
REG_FIDDLE(Canvas_makeSurface, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    sk_sp<SkSurface> surface = SkSurface::MakeRasterN32Premul(5, 6);
    SkCanvas* smallCanvas = surface->getCanvas();
    SkImageInfo imageInfo = SkImageInfo::MakeN32Premul(3, 4);
    sk_sp<SkSurface> compatible = smallCanvas->makeSurface(imageInfo);
    SkDebugf("compatible %c= nullptr\n", compatible == nullptr ? '=' : '!');
    SkDebugf("size = %d, %d\n", compatible->width(), compatible->height());
}
}  // END FIDDLE
