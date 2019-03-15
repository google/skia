#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=4b3d879118ef770d1f11a23c6493b2c4
REG_FIDDLE(Picture_010, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPictureRecorder recorder;
    SkCanvas* pictureCanvas = recorder.beginRecording({0, 0, 256, 256});
    SkPaint paint;
    pictureCanvas->drawRect(SkRect::MakeWH(200, 200), paint);
    paint.setColor(SK_ColorWHITE);
    pictureCanvas->drawRect(SkRect::MakeLTRB(20, 20, 180, 180), paint);
    sk_sp<SkPicture> picture = recorder.finishRecordingAsPicture();
    picture->playback(canvas);
    std::string opCount = "approximate op count: " + std::to_string(picture->approximateOpCount());
    canvas->drawString(opCount.c_str(), 50, 220, SkPaint());
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
