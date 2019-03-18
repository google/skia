// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=069c7b116479e3ca46f953f07dcbdd36
REG_FIDDLE(Image_MakeCrossContextFromEncoded, 256, 64, false, 4) {
void draw(SkCanvas* canvas) {
    GrContext* context = canvas->getGrContext();
    sk_sp<SkData> encodedData = image->encodeToData(SkEncodedImageFormat::kJPEG, 100);
    sk_sp<SkImage> image = SkImage::MakeCrossContextFromEncoded(context,
                                                                encodedData, false, nullptr);
    canvas->drawImage(image, 0, 0);
}
}  // END FIDDLE
