// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
namespace {
REG_FIDDLE(Surface_012, 256, 256, true, 0);
// HASH=20571cc23e3146deaa09046b64cc0aef
void draw(SkCanvas* canvas) {
    const int width = 37;
    const int height = 1000;
    auto surf = SkSurface::MakeNull(width, height);
    auto nullCanvas = surf->getCanvas();
    SkDebugf("surface height=%d  canvas height=%d\n", surf->height(),
             nullCanvas->getBaseLayerSize().fHeight);
}
}
