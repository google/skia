// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=58b44bf47d8816782066618700afdecb
REG_FIDDLE(Picture_002, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPictureRecorder recorder;
    SkCanvas* pictureCanvas = recorder.beginRecording({0, 0, 256, 256});
    SkPaint paint;
    pictureCanvas->drawRect(SkRect::MakeWH(200, 200), paint);
    paint.setColor(SK_ColorWHITE);
    pictureCanvas->drawRect(SkRect::MakeLTRB(20, 20, 180, 180), paint);
    sk_sp<SkPicture> picture = recorder.finishRecordingAsPicture();
    SkDynamicMemoryWStream writableStream;
    picture->serialize(&writableStream);
    sk_sp<SkData> readableData = writableStream.detachAsData();
    sk_sp<SkPicture> copy = SkPicture::MakeFromData(readableData.get());
    copy->playback(canvas);
}
}  // END FIDDLE
