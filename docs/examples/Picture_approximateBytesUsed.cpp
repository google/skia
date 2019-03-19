#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=ececbda21218bd732394a305dba393a2
REG_FIDDLE(Picture_approximateBytesUsed, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPictureRecorder recorder;
    SkCanvas* pictureCanvas = recorder.beginRecording({0, 0, 256, 256});
    SkPaint paint;
    pictureCanvas->drawRect(SkRect::MakeWH(200, 200), paint);
    paint.setColor(SK_ColorWHITE);
    pictureCanvas->drawRect(SkRect::MakeLTRB(20, 20, 180, 180), paint);
    sk_sp<SkPicture> picture = recorder.finishRecordingAsPicture();
    picture->playback(canvas);
    std::string opCount = "approximate bytes used: " + std::to_string(picture->approximateBytesUsed());
    canvas->drawString(opCount.c_str(), 20, 220, SkPaint());
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
