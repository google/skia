// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
namespace {
REG_FIDDLE(Surface_025, 256, 96, false, 4);
// HASH=d77790dd3bc9f678fa4f582347fb8fba
void draw(SkCanvas* canvas) {
    sk_sp<SkSurface> surf(SkSurface::MakeRasterN32Premul(64, 64));
    auto surfCanvas = surf->getCanvas();
    surfCanvas->clear(SK_ColorGREEN);
    surf->writePixels(source, 25, 25);
    sk_sp<SkImage> image(surf->makeImageSnapshot());
    canvas->drawImage(image, 0, 0);
}
}
