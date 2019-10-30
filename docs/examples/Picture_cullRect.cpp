// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=15bb9a9596b40c5e2045f76e8c1dcf8e
REG_FIDDLE(Picture_cullRect, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPictureRecorder recorder;
    SkCanvas* pictureCanvas = recorder.beginRecording({64, 64, 192, 192});
    SkPaint paint;
    pictureCanvas->drawRect(SkRect::MakeWH(200, 200), paint);
    paint.setColor(SK_ColorWHITE);
    pictureCanvas->drawRect(SkRect::MakeLTRB(20, 20, 180, 180), paint);
    sk_sp<SkPicture> picture = recorder.finishRecordingAsPicture();
    picture->playback(canvas);
    paint.setBlendMode(SkBlendMode::kModulate);
    paint.setColor(0x40404040);
    canvas->drawRect(picture->cullRect(), paint);
}
}  // END FIDDLE
