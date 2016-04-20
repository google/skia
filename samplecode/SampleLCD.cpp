/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkShader.h"

class LCDView : public SkView {
public:
    LCDView() {}

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "LCD Text");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(SK_ColorWHITE);
    }

    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);

        SkPaint paint;
        paint.setAntiAlias(true);

        SkScalar textSize = SkIntToScalar(6);
        SkScalar delta = SK_Scalar1;
        const char* text = "HHHamburgefonts iii";
        size_t len = strlen(text);
        SkScalar x0 = SkIntToScalar(10);
        SkScalar x1 = SkIntToScalar(310);
        SkScalar y = SkIntToScalar(20);

        for (int i = 0; i < 20; i++) {
            paint.setTextSize(textSize);
            textSize += delta;

            paint.setLCDRenderText(false);
            canvas->drawText(text, len, x0, y, paint);
            paint.setLCDRenderText(true);
            canvas->drawText(text, len, x1, y, paint);

            y += paint.getFontSpacing();
        }
    }

private:
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new LCDView; }
static SkViewRegister reg(MyFactory);
