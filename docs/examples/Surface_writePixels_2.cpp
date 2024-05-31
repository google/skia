// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Surface_writePixels_2, 256, 96, false, 4) {
void draw(SkCanvas* canvas) {
    sk_sp<SkSurface> surf(SkSurfaces::Raster(SkImageInfo::MakeN32Premul(64, 64)));
    auto surfCanvas = surf->getCanvas();
    surfCanvas->clear(SK_ColorGREEN);
    surf->writePixels(source, 25, 25);
    sk_sp<SkImage> image(surf->makeImageSnapshot());
    canvas->drawImage(image, 0, 0);
}
}  // END FIDDLE
