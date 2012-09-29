/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "gm.h"
#include "SkCanvas.h"

namespace skiagm {

#define TEXT_SIZE   48
static const char gText[] = "Hello";
static const size_t gLen = sizeof(gText) - 1;

class VertTextGM : public GM {
public:
    VertTextGM() {}

protected:

    SkString onShortName() {
        return SkString("verttext");
    }

    SkISize onISize() { return make_isize(640, 480); }

    static void drawBaseline(SkCanvas* canvas, const SkPaint& paint,
                             SkScalar x, SkScalar y) {
        SkScalar total = paint.measureText(gText, gLen);

        SkPaint p;
        p.setAntiAlias(true);
        p.setColor(0x80FF0000);
        canvas->drawLine(x, y,
                         paint.isVerticalText() ? x : x + total,
                         paint.isVerticalText() ? y + total : y,
                         p);

        p.setColor(0xFF0000FF);
        SkScalar adv[gLen];
        paint.getTextWidths(gText, gLen, adv, NULL);
        for (size_t i = 0; i < gLen; ++i) {
            canvas->drawCircle(x, y, SK_Scalar1 * 3 / 2, p);
            if (paint.isVerticalText()) {
                y += adv[i];
            } else {
                x += adv[i];
            }
        }
        canvas->drawCircle(x, y, SK_Scalar1 * 3 / 2, p);
    }

    virtual void onDraw(SkCanvas* canvas) {
        SkScalar x = SkIntToScalar(100);
        SkScalar y = SkIntToScalar(50);

        for (int i = 0; i < 4; ++i) {
            SkPaint     paint;
            paint.setAntiAlias(true);
            paint.setTextSize(SkIntToScalar(TEXT_SIZE));


            paint.setVerticalText(false);
            drawBaseline(canvas, paint, x, y);
            canvas->drawText(gText, gLen, x, y, paint);

            paint.setVerticalText(true);
            drawBaseline(canvas, paint, x, y);
            canvas->drawText(gText, gLen, x, y, paint);

            x += SkIntToScalar(40);
            y += SkIntToScalar(120);

            canvas->rotate(SkIntToScalar(-15));
        }
    }

private:
    typedef GM INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new VertTextGM; }
static GMRegistry reg(MyFactory);

}
