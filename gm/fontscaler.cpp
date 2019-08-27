/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"

#include <string.h>

namespace skiagm {

class FontScalerGM : public GM {
public:
    FontScalerGM() {
        this->setBGColor(0xFFFFFFFF);
    }

protected:

    SkString onShortName() override {
        return SkString("fontscaler");
    }

    SkISize onISize() override {
        return SkISize::Make(1450, 750);
    }

    void onDraw(SkCanvas* canvas) override {
        SkFont font;
        font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
        //With freetype the default (normal hinting) can be really ugly.
        //Most distros now set slight (vertical hinting only) in any event.
        font.setHinting(SkFontHinting::kSlight);

        const char* text = "Hamburgefons ooo mmm";
        const size_t textLen = strlen(text);

        for (int j = 0; j < 2; ++j) {
            // This used to do 6 iterations but it causes the N4 to crash in the MSAA4 config.
            for (int i = 0; i < 5; ++i) {
                SkScalar x = 10;
                SkScalar y = 20;

                SkAutoCanvasRestore acr(canvas, true);
                canvas->translate(SkIntToScalar(50 + i * 230),
                                  SkIntToScalar(20));
                canvas->rotate(SkIntToScalar(i * 5), x, y * 10);

                {
                    SkPaint p;
                    p.setAntiAlias(true);
                    SkRect r;
                    r.setLTRB(x - 3, 15, x - 1, 280);
                    canvas->drawRect(r, p);
                }

                for (int ps = 6; ps <= 22; ps++) {
                    font.setSize(SkIntToScalar(ps));
                    canvas->drawSimpleText(text, textLen, SkTextEncoding::kUTF8, x, y, font, SkPaint());
                    y += font.getMetrics(nullptr);
                }
            }
            canvas->translate(0, SkIntToScalar(360));
            font.setSubpixel(true);
            font.setLinearMetrics(true);
            font.setBaselineSnap(false);
        }
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new FontScalerGM; )

}
