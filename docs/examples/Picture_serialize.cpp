// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=dacdebe1355c884ebd3c2ea038cc7a20
REG_FIDDLE(Picture_serialize, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPictureRecorder recorder;
    SkCanvas* pictureCanvas = recorder.beginRecording({0, 0, 256, 256});
    SkPaint paint;
    pictureCanvas->drawRect(SkRect::MakeWH(200, 200), paint);
    paint.setColor(SK_ColorWHITE);
    pictureCanvas->drawRect(SkRect::MakeLTRB(20, 20, 180, 180), paint);
    sk_sp<SkPicture> picture = recorder.finishRecordingAsPicture();
    sk_sp<SkData> readableData = picture->serialize();
    sk_sp<SkPicture> copy = SkPicture::MakeFromData(readableData->data(), readableData->size());
    copy->playback(canvas);
}
}  // END FIDDLE
