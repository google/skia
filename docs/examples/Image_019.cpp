#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=4468d573f42af6f5e234be10a5453bb2
REG_FIDDLE(Image_019, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    SkPixmap pixmap;
    source.peekPixels(&pixmap);
    canvas->scale(.25f, .25f);
    int y = 0;
    for (auto gamma : { SkColorSpace::kLinear_RenderTargetGamma,
                        SkColorSpace::kSRGB_RenderTargetGamma } ) {
        int x = 0;
        sk_sp<SkColorSpace> colorSpace = SkColorSpace::MakeRGB(gamma, SkColorSpace::kSRGB_Gamut);
        for (int index = 0; index < 2; ++index) {
            pixmap.setColorSpace(colorSpace);
            sk_sp<SkImage> image = SkImage::MakeRasterCopy(pixmap);
            canvas->drawImage(image, x, y);
            colorSpace = image->colorSpace()->makeColorSpin();
            x += 512;
        }
        y += 512;
    }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
