/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Sample.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkShader.h"

class LCDView : public Sample {
public:
    LCDView() {}

protected:
    bool onQuery(Sample::Event* evt) override {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, "LCD Text");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(SK_ColorWHITE);
    }

    void onDrawContent(SkCanvas* canvas) override {
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
    typedef Sample INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new LCDView(); )
