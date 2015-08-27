
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkShader.h"
#include "SkKey.h"

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

static void make_paint(SkPaint* paint, SkShader::TileMode tm) {
    SkBitmap bm;
    make_bitmap(&bm);

    SkShader* shader = SkShader::CreateBitmapShader(bm, tm, tm);
    paint->setShader(shader)->unref();
}

class RepeatTileView : public SampleView {
public:
    RepeatTileView() {
        this->setBGColor(SK_ColorGRAY);
    }

protected:
    // overrides from SkEventSink
    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "RepeatTile");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void onDrawContent(SkCanvas* canvas) override {
        SkPaint paint;
        make_paint(&paint, SkShader::kRepeat_TileMode);

//        canvas->scale(SK_Scalar1*2, SK_Scalar1);
        canvas->translate(SkIntToScalar(100), SkIntToScalar(100));
        canvas->drawPaint(paint);
    }

    SkView::Click* onFindClickHandler(SkScalar x, SkScalar y, unsigned modi) override {
        this->inval(nullptr);

        return this->INHERITED::onFindClickHandler(x, y, modi);
    }

    bool onClick(Click* click) override {
        return this->INHERITED::onClick(click);
    }

    virtual bool handleKey(SkKey) {
        this->inval(nullptr);
        return true;
    }

private:
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new RepeatTileView; }
static SkViewRegister reg(MyFactory);
