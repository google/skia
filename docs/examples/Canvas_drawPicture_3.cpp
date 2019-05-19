// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=759e4e5bac680838added8f70884dcdc
REG_FIDDLE(Canvas_drawPicture_3, 256, 256, false, 0) {
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
    const SkPicture* playbackPtr = playback.get();
    SkMatrix matrix;
    matrix.reset();
    for (auto alpha : { 70, 140, 210 } ) {
    paint.setAlpha(alpha);
    canvas->drawPicture(playbackPtr, &matrix, &paint);
    matrix.preTranslate(70, 70);
    }
}
}  // END FIDDLE
