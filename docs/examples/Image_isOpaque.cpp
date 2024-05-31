// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Image_isOpaque, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto check_isopaque = [](const SkImageInfo& imageInfo) -> void {
        auto surface(SkSurfaces::Raster(imageInfo));
        auto image(surface->makeImageSnapshot());
        SkDebugf("isOpaque = %s\n", image->isOpaque() ? "true" : "false");
    };
    check_isopaque(SkImageInfo::MakeN32Premul(5, 5));
    check_isopaque(SkImageInfo::MakeN32(5, 5, kOpaque_SkAlphaType));
}
}  // END FIDDLE
