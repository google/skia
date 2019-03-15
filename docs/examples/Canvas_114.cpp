// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=83918a23fcffd47f59a1ef662c85a24c
REG_FIDDLE(Canvas_114, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPictureRecorder recorder;
    SkCanvas* recordingCanvas = recorder.beginRecording(50, 50);
    for (auto color : { SK_ColorRED, SK_ColorBLUE, 0xff007f00 } ) {
        SkPaint paint;
        paint.setColor(color);
        recordingCanvas->drawRect({10, 10, 30, 40}, paint);
        recordingCanvas->translate(10, 10);
        recordingCanvas->scale(1.2f, 1.4f);
    }
    sk_sp<SkPicture> playback = recorder.finishRecordingAsPicture();
    canvas->drawPicture(playback);
    canvas->scale(2, 2);
    canvas->translate(50, 0);
    canvas->drawPicture(playback);
}
}  // END FIDDLE
