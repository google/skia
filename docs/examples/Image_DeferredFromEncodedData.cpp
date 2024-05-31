// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Image_DeferredFromEncodedData, 256, 256, false, 3) {
    void draw(SkCanvas * canvas) {
        int x = 0;
        for (int quality : {100, 50, 10, 1}) {
            SkJpegEncoder::Options options;
            options.fQuality = quality;
            sk_sp<SkData> data(SkJpegEncoder::Encode(nullptr, image.get(), options));
            sk_sp<SkImage> image = SkImages::DeferredFromEncodedData(data);
            canvas->drawImage(image, x, 0);
            x += 64;
        }
    }
}  // END FIDDLE
