/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkDashPathEffect.h"

static void drawline(SkCanvas* canvas, int on, int off, const SkPaint& paint) {
    SkPaint p(paint);

    const SkScalar intervals[] = {
        SkIntToScalar(on),
        SkIntToScalar(off),
    };

    p.setPathEffect(new SkDashPathEffect(intervals, 2, 0))->unref();
    canvas->drawLine(0, 0, SkIntToScalar(600), 0, p);
}

namespace skiagm {

class DashingGM : public GM {
public:
    DashingGM() {}

protected:
    SkString onShortName() {
        return SkString("dashing");
    }

    SkISize onISize() { return make_isize(640, 300); }

    virtual void onDraw(SkCanvas* canvas) {
        static const struct {
            int fOnInterval;
            int fOffInterval;
        } gData[] = {
            { 1, 1 },
            { 4, 1 },
        };
        
        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);

        canvas->translate(SkIntToScalar(20), SkIntToScalar(20));
        canvas->translate(0, SK_ScalarHalf);

        for (int width = 0; width <= 2; ++width) {
            for (size_t data = 0; data < SK_ARRAY_COUNT(gData); ++data) {
                for (int aa = 0; aa <= 1; ++aa) {
                    int w = width * width * width;
                    paint.setAntiAlias(SkToBool(aa));
                    paint.setStrokeWidth(SkIntToScalar(w));
                    
                    int scale = w ? w : 1;

                    drawline(canvas, gData[data].fOnInterval * scale,
                             gData[data].fOffInterval * scale,
                             paint);
                    canvas->translate(0, SkIntToScalar(20));
                }
            }
        }
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* gF(void*) { return new DashingGM; }
static GMRegistry gR(gF);

}
