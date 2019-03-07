// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=30cee813f6aa476b0a9c8a24283e53a3
REG_FIDDLE(Image_033, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    canvas->scale(4, 4);
    SkIRect subset = {136, 32, 200, 96};
    sk_sp<SkData> data(image->encodeToData());
    sk_sp<SkImage> eye = SkImage::MakeFromEncoded(data, &subset);
    canvas->drawImage(eye, 0, 0);
}
}  // END FIDDLE
