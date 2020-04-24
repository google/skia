// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=894f732ed6409b1f392bc5481421d0e9
REG_FIDDLE(Image_MakeFromEncoded, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    int x = 0;
    for (int quality : { 100, 50, 10, 1} ) {
        sk_sp<SkData> encodedData = image->encodeToData(SkEncodedImageFormat::kJPEG, quality);
        sk_sp<SkImage> image = SkImage::MakeFromEncoded(encodedData);
        canvas->drawImage(image, x, 0);
        x += 64;
    }
}
}  // END FIDDLE
