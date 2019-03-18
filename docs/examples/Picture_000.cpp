// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=56ed920dadbf2b2967ac45fb5a9bded6
REG_FIDDLE(Picture_AbortCallback_abort, 256, 256, false, 0) {
class JustOneDraw : public SkPicture::AbortCallback {
public:
    bool abort() override { return fCalls++ > 0; }
private:
    int fCalls = 0;
};

void draw(SkCanvas* canvas) {
    SkPictureRecorder recorder;
    SkCanvas* pictureCanvas = recorder.beginRecording({0, 0, 256, 256});
    SkPaint paint;
    pictureCanvas->drawRect(SkRect::MakeWH(200, 200), paint);
    paint.setColor(SK_ColorWHITE);
    pictureCanvas->drawRect(SkRect::MakeLTRB(20, 20, 180, 180), paint);
    sk_sp<SkPicture> picture = recorder.finishRecordingAsPicture();
    JustOneDraw callback;
    picture->playback(canvas, &callback);
}
}  // END FIDDLE
