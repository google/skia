/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkShader.h"
#include "samplecode/Sample.h"

class LCDView : public Sample {
public:
    LCDView() {}

protected:
    SkString name() override { return SkString("LCD Text"); }

    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(SK_ColorWHITE);
    }

    void onDrawContent(SkCanvas* canvas) override {
        this->drawBG(canvas);

        SkPaint paint;

        SkScalar textSize = SkIntToScalar(6);
        SkScalar delta = SK_Scalar1;
        const char* text = "HHHamburgefonts iii";
        size_t len = strlen(text);
        SkScalar x0 = SkIntToScalar(10);
        SkScalar x1 = SkIntToScalar(310);
        SkScalar y = SkIntToScalar(20);

        SkFont font;
        for (int i = 0; i < 20; i++) {
            font.setSize(textSize);
            textSize += delta;

            font.setEdging(SkFont::Edging::kAntiAlias);
            canvas->drawSimpleText(text, len, SkTextEncoding::kUTF8, x0, y, font, paint);
            font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
            canvas->drawSimpleText(text, len, SkTextEncoding::kUTF8, x1, y, font, paint);

            y += font.getSpacing();
        }
    }

private:
    typedef Sample INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new LCDView(); )
