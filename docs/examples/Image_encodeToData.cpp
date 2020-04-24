// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=7a3bf8851bb7160e4e49c48f8c09639d
REG_FIDDLE(Image_encodeToData, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    canvas->scale(4, 4);
    SkIRect subset = {0, 0, 16, 64};
    int x = 0;
    for (int quality : { 0, 10, 50, 100 } ) {
        sk_sp<SkData> data(image->encodeToData(SkEncodedImageFormat::kJPEG, quality));
        sk_sp<SkImage> filtered = SkImage::MakeFromEncoded(data, &subset);
        canvas->drawImage(filtered, x, 0);
        x += 16;
    }
}
}  // END FIDDLE
