// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=4aa2879b9e44dfd6648995326d2c4dcf
REG_FIDDLE(Image_MakeFromPicture, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkPictureRecorder recorder;
    SkCanvas* recordingCanvas = recorder.beginRecording(50, 50);
    for (auto color : { SK_ColorRED, SK_ColorBLUE, 0xff007f00 } ) {
        paint.setColor(color);
        recordingCanvas->drawRect({10, 10, 30, 40}, paint);
        recordingCanvas->translate(10, 10);
        recordingCanvas->scale(1.2f, 1.4f);
    }
    sk_sp<SkPicture> playback = recorder.finishRecordingAsPicture();
    int x = 0, y = 0;
    for (auto alpha : { 70, 140, 210 } ) {
        paint.setAlpha(alpha);
        auto srgbColorSpace = SkColorSpace::MakeSRGB();
        sk_sp<SkImage> image = SkImage::MakeFromPicture(playback, {50, 50}, nullptr, &paint,
                                                        SkImage::BitDepth::kU8, srgbColorSpace);
        canvas->drawImage(image, x, y);
        x += 70; y += 70;
    }
}
}  // END FIDDLE
