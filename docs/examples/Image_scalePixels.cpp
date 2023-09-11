// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=5949c9a63610cae30019e5b1899ee38f
REG_FIDDLE(Image_scalePixels, 256, 128, false, 3) {
void draw(SkCanvas* canvas) {
    std::vector<int32_t> srcPixels;
    int quarterWidth = image->width() / 16;
    int rowBytes = quarterWidth * 4;
    int quarterHeight = image->height() / 16;
    srcPixels.resize(quarterHeight * rowBytes);
    SkPixmap pixmap(SkImageInfo::MakeN32Premul(quarterWidth, quarterHeight),
                    &srcPixels.front(), rowBytes);
    canvas->scale(4, 4);

    const SkSamplingOptions samplings[] = {
        SkSamplingOptions(),
        SkSamplingOptions(SkFilterMode::kLinear),
        SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kLinear),
        SkSamplingOptions({1.0f/3, 1.0f/3}),
    };
    for (unsigned index = 0; index < std::size(samplings); ++index) {
        image->scalePixels(pixmap, samplings[index]);
        sk_sp<SkImage> filtered = SkImages::RasterFromPixmap(pixmap, nullptr, nullptr);
        canvas->drawImage(filtered, 16 * index, 0);
    }
}
}  // END FIDDLE
