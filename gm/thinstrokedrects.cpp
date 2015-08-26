/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

namespace skiagm {

// Draw rects with various stroke widths at 1/8 pixel increments
class ThinStrokedRectsGM : public GM {
public:
    ThinStrokedRectsGM() {
        this->setBGColor(0xFF000000);
    }

protected:
    SkString onShortName() override {
        return SkString("thinstrokedrects");
    }

    SkISize onISize() override {
        return SkISize::Make(240, 320);
    }

    void onDraw(SkCanvas* canvas) override {

        SkPaint paint;
        paint.setColor(SK_ColorWHITE);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setAntiAlias(true);

        static const SkRect rect = { 0, 0, 10, 10 };
        static const SkRect rect2 = { 0, 0, 20, 20 };

        static const SkScalar gStrokeWidths[] = {
            4, 2, 1, 0.5f, 0.25f, 0.125f, 0
        };

        canvas->translate(5, 5);
        for (int i = 0; i < 8; ++i) {
            canvas->save();
            canvas->translate(i*0.125f, i*30.0f);
            for (size_t j = 0; j < SK_ARRAY_COUNT(gStrokeWidths); ++j) {
                paint.setStrokeWidth(gStrokeWidths[j]);
                canvas->drawRect(rect, paint);
                canvas->translate(15, 0);
            }
            canvas->restore();
        }

        // Draw a second time in red with a scale
        paint.setColor(SK_ColorRED);
        canvas->translate(0, 15);
        for (int i = 0; i < 8; ++i) {
            canvas->save();
            canvas->translate(i*0.125f, i*30.0f);
            canvas->scale(0.5f, 0.5f);
            for (size_t j = 0; j < SK_ARRAY_COUNT(gStrokeWidths); ++j) {
                paint.setStrokeWidth(2.0f * gStrokeWidths[j]);
                canvas->drawRect(rect2, paint);
                canvas->translate(30, 0);
            }
            canvas->restore();
        }
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ThinStrokedRectsGM;)
}
