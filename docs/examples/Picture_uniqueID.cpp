// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=8e4257245c988c600410fe4fd7293f07
REG_FIDDLE(Picture_uniqueID, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPictureRecorder recorder;
    recorder.beginRecording({0, 0, 0, 0});
    sk_sp<SkPicture> picture = recorder.finishRecordingAsPicture();
    SkDebugf("empty picture id = %d\n", picture->uniqueID());
    sk_sp<SkPicture> placeholder = SkPicture::MakePlaceholder({0, 0, 0, 0});
    SkDebugf("placeholder id = %d\n", placeholder->uniqueID());
}
}  // END FIDDLE
