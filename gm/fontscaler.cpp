/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include <SkFont.h>
#include "gm.h"
#include "sk_tool_utils.h"
#include "SkTypeface.h"

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
        font.setHinting(kSlight_SkFontHinting);

        const char* text = "Hamburgefons ooo mmm";
        const size_t textLen = strlen(text);

        for (int j = 0; j < 2; ++j) {
            // This used to do 6 iterations but it causes the N4 to crash in the MSAA4 config.
            for (int i = 0; i < 5; ++i) {
                SkScalar x = SkIntToScalar(10);
                SkScalar y = SkIntToScalar(20);

                SkAutoCanvasRestore acr(canvas, true);
                canvas->translate(SkIntToScalar(50 + i * 230),
                                  SkIntToScalar(20));
                canvas->rotate(SkIntToScalar(i * 5), x, y * 10);

                {
                    SkPaint p;
                    p.setAntiAlias(true);
                    SkRect r;
                    r.set(x - SkIntToScalar(3), SkIntToScalar(15),
                          x - SkIntToScalar(1), SkIntToScalar(280));
                    canvas->drawRect(r, p);
                }

                for (int ps = 6; ps <= 22; ps++) {
                    font.setSize(SkIntToScalar(ps));
                    canvas->drawSimpleText(text, textLen, kUTF8_SkTextEncoding, x, y, font, SkPaint());
                    y += font.getMetrics(nullptr);
                }
            }
            canvas->translate(0, SkIntToScalar(360));
            font.setSubpixel(true);
        }
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new FontScalerGM; )

}
