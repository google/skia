/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "samplecode/DecodeFile.h"
#include "samplecode/Sample.h"

namespace{
struct LinesView : public Sample {
    SkString name() override { return SkString("Lines"); }
    void onDrawContent(SkCanvas* canvas) override {
        SkBitmap bm;
        // TODO: replace with Resource.
        if (decode_file("/kill.gif", &bm)) {
            canvas->drawBitmap(bm, 0, 0, nullptr);
        }
        canvas->scale(0.5f, 0.5f);
        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(1.5f);
        paint.setColor(SkColorSetARGB(0xFF, 0xFF, 0x88, 0x00));
        canvas->drawRect(SkRect{10, 10, 110, 110}, paint);
    }
};
}
DEF_SAMPLE( return new LinesView(); )
