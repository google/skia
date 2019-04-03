/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Sample.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkShader.h"

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

    paint->setShader(SkShader::MakeBitmapShader(bm, tm, tm));
}

class RepeatTileView : public Sample {
public:
    RepeatTileView() {
        this->setBGColor(SK_ColorGRAY);
    }

protected:
    bool onQuery(Sample::Event* evt) override {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, "RepeatTile");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void onDrawContent(SkCanvas* canvas) override {
        SkPaint paint;
        make_paint(&paint, SkTileMode::kRepeat);

//        canvas->scale(SK_Scalar1*2, SK_Scalar1);
        canvas->translate(SkIntToScalar(100), SkIntToScalar(100));
        canvas->drawPaint(paint);
    }

private:
    typedef Sample INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new RepeatTileView(); )
