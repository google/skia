// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Image_encodeToData, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    canvas->scale(4, 4);
    SkIRect subset = {0, 0, 16, 64};
    int x = 0;
    for (int quality : { 0, 10, 50, 100 } ) {
        SkJpegEncoder::Options options;
        options.fQuality = quality;
        sk_sp<SkData> data(SkJpegEncoder::Encode(nullptr, image.get(), options));
        sk_sp<SkImage> filtered = SkImages::DeferredFromEncodedData(data)->
                makeSubset(nullptr, subset);
        canvas->drawImage(filtered, x, 0);
        x += 16;
    }
}
}  // END FIDDLE
