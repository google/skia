/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "skia.h"

static void draw(SkCanvas* canvas);
DEF_SIMPLE_GM(fiddle, canvas, 256, 256) { draw(canvas); }

// Paste your fiddle.skia.org code over this stub.
void draw(SkCanvas* canvas) {
    std::vector<uint8_t> rgbx_texels = {255, 0, 255, 128, 255, 0, 255, 64,
                                        255, 0, 255,  13, 255, 0, 255, 14};
    SkImageInfo rgbx_info = SkImageInfo::Make(2, 2, kRGB_888x_SkColorType, kOpaque_SkAlphaType);
    SkPixmap rgbx_pixmap(rgbx_info, rgbx_texels.data(), 8u);
    sk_sp<SkImage> rgbx_image = SkImage::MakeFromRaster(rgbx_pixmap, nullptr, nullptr);
    rgbx_image = rgbx_image->makeTextureImage(canvas->getGrContext());
    canvas->scale(20, 20);
    canvas->drawImage(rgbx_image, 0, 0);
}
