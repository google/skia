// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=0d2cbf82f490ffb180e0b4531afa232c
REG_FIDDLE(Picture_MakePlaceholder, 256, 256, false, 0) {
class MyCanvas : public SkCanvas {
public:
    MyCanvas(SkCanvas* c) : canvas(c) {}
        void onDrawPicture(const SkPicture* picture, const SkMatrix* ,
                               const SkPaint* ) override {
        const SkRect rect = picture->cullRect();
        SkPaint redPaint;
        redPaint.setColor(SK_ColorRED);
        canvas->drawRect(rect, redPaint);
   }
   SkCanvas* canvas;
};

void draw(SkCanvas* canvas) {
    SkPictureRecorder recorder;
    SkCanvas* pictureCanvas = recorder.beginRecording({0, 0, 256, 256});
    sk_sp<SkPicture> placeholder = SkPicture::MakePlaceholder({10, 40, 80, 110});
    pictureCanvas->drawPicture(placeholder);
    sk_sp<SkPicture> picture = recorder.finishRecordingAsPicture();
    MyCanvas myCanvas(canvas);
    myCanvas.drawPicture(picture);
}
}  // END FIDDLE
