/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkShader.h"
#include "samplecode/Sample.h"

static void make_bitmap(SkBitmap* bm) {
    const int W = 100;
    const int H = 100;
    bm->allocN32Pixels(W, H);

    SkPaint paint;
    SkCanvas canvas(*bm);
    canvas.drawColor(SK_ColorWHITE);

    const SkColor colors[] = {
        SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorWHITE
    };

    for (int ix = 0; ix < W; ix += 1) {
        SkScalar x = SkIntToScalar(ix) + SK_ScalarHalf;
        paint.setColor(colors[ix & 3]);
        canvas.drawLine(x, 0, x, SkIntToScalar(H - 1), paint);
    }
    paint.setColor(SK_ColorGRAY);
    canvas.drawLine(0, 0, SkIntToScalar(W), 0, paint);
}

static void make_paint(SkPaint* paint, SkTileMode tm) {
    SkBitmap bm;
    make_bitmap(&bm);

    paint->setShader(bm.makeShader(tm, tm, SkSamplingOptions()));
}

class RepeatTileView : public Sample {
public:
    RepeatTileView() {
        this->setBGColor(SK_ColorGRAY);
    }

protected:
    SkString name() override { return SkString("RepeatTile"); }

    void onDrawContent(SkCanvas* canvas) override {
        SkPaint paint;
        make_paint(&paint, SkTileMode::kRepeat);

//        canvas->scale(SK_Scalar1*2, SK_Scalar1);
        canvas->translate(SkIntToScalar(100), SkIntToScalar(100));
        canvas->drawPaint(paint);
    }

private:
    using INHERITED = Sample;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new RepeatTileView(); )
