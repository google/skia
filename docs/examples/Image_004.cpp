// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=c2fec0746f88ca34d7dce59dd9bdef9e
REG_FIDDLE(Image_MakeFromGenerator, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPictureRecorder recorder;
    recorder.beginRecording(100, 100)->drawColor(SK_ColorRED);
    auto picture = recorder.finishRecordingAsPicture();
    auto gen = SkImageGenerator::MakeFromPicture({100, 100}, picture, nullptr, nullptr,
                                                 SkImage::BitDepth::kU8, SkColorSpace::MakeSRGB());
    sk_sp<SkImage> image = SkImage::MakeFromGenerator(std::move(gen));
    canvas->drawImage(image, 0, 0);
}
}  // END FIDDLE
