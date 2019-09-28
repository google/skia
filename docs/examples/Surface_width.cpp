// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=df066b56dd97c7c589fd2bb6a2539de8
REG_FIDDLE(Surface_width, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    const int width = 37;
    const int height = 1000;
    auto surf = SkSurface::MakeNull(width, height);
    auto nullCanvas = surf->getCanvas();
    SkDebugf("surface width=%d  canvas width=%d\n", surf->width(),
             nullCanvas->getBaseLayerSize().fWidth);
}
}  // END FIDDLE
