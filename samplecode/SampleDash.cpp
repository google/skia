
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkGraphics.h"
#include "SkRandom.h"
#include "SkDashPathEffect.h"
#include "SkShader.h"

static void setBitmapDash(SkPaint* paint, int width) {
    SkColor c = paint->getColor();

    SkBitmap bm;
    bm.allocN32Pixels(2, 1);
    bm.lockPixels();
    *bm.getAddr32(0, 0) = SkPreMultiplyARGB(0xFF, SkColorGetR(c),
                                            SkColorGetG(c), SkColorGetB(c));
    *bm.getAddr32(1, 0) = 0;
    bm.unlockPixels();

    SkMatrix matrix;
    matrix.setScale(SkIntToScalar(width), SK_Scalar1);

    SkShader* s = SkShader::CreateBitmapShader(bm, SkShader::kRepeat_TileMode,
                                               SkShader::kClamp_TileMode, &matrix);

    paint->setShader(s)->unref();
}

class DashView : public SampleView {
public:
    DashView() {
        this->setBGColor(0xFFDDDDDD);
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Dash");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        static const char* gStr[] = {
            "11",
            "44",
            "112233",
            "411327463524",
        };

        SkPaint paint;
        paint.setStrokeWidth(SkIntToScalar(1));

        SkScalar x0 = SkIntToScalar(10);
        SkScalar y0 = SkIntToScalar(10);
        SkScalar x1 = x0 + SkIntToScalar(1000);
        for (size_t i = 0; i < SK_ARRAY_COUNT(gStr); i++) {
            SkScalar interval[12];
            size_t len = SkMin32(strlen(gStr[i]), SK_ARRAY_COUNT(interval));
            for (size_t j = 0; j < len; j++) {
                interval[j] = SkIntToScalar(gStr[i][j] - '0');
            }

            SkDashPathEffect dash(interval, len, 0);
            paint.setPathEffect(&dash);
            canvas->drawLine(x0, y0, x1, y0, paint);
            paint.setPathEffect(nullptr);

            y0 += paint.getStrokeWidth() * 3;
        }

        setBitmapDash(&paint, 3);
        canvas->drawLine(x0, y0, x1, y0, paint);
    }

private:
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new DashView; }
static SkViewRegister reg(MyFactory);
