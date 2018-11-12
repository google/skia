/*
* Copyright 2018 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "gm.h"
#include "sk_tool_utils.h"

#include "Resources.h"
#include "SkCanvas.h"
#include "SkStream.h"
#include "SkTo.h"
#include "SkTypeface.h"

namespace skiagm {
class ScaledEmojiRenderingGM : public GM {
public:
    ScaledEmojiRenderingGM() {}

protected:
    sk_sp<SkTypeface> typefaces[4];

    void onOnceBeforeDraw() override {
        typefaces[0] = MakeResourceAsTypeface("fonts/colr.ttf");
        typefaces[1] = MakeResourceAsTypeface("fonts/sbix.ttf");
        typefaces[2] = MakeResourceAsTypeface("fonts/cbdt.ttf");
        typefaces[3] = sk_tool_utils::create_portable_typeface("Emoji", SkFontStyle());
    }

    SkString onShortName() override {
        SkString name("scaledemoji_rendering");
        name.append(sk_tool_utils::platform_font_manager());
        return name;
    }

    SkISize onISize() override { return SkISize::Make(1200, 1200); }

    void onDraw(SkCanvas* canvas) override {

        canvas->drawColor(SK_ColorGRAY);
        SkScalar y = 0;

        for (const auto& typeface: typefaces) {
            SkPaint paint;
            paint.setTypeface(typeface);
            const char* text = sk_tool_utils::emoji_sample_text();
            SkFontMetrics metrics;

            for (SkScalar textSize : { 70, 150 }) {
                paint.setTextSize(textSize);
                paint.getFontMetrics(&metrics);
                // All typefaces should support subpixel mode
                paint.setSubpixelText(true);
                y += -metrics.fAscent;

                int len = SkToInt(strlen(text));
                SkAutoTArray<SkPoint>  pos(len);
                SkAutoTArray<SkScalar> widths(len);
                int found = paint.getTextWidths(text, len, &widths[0]);
                SkScalar x = SkIntToScalar(10);
                for (int i = 0; i < found; ++i) {
                    pos[i].set(x, y);
                    x += widths[i];
                }

                canvas->drawPosText(text, len, &pos[0], paint);
                y += metrics.fDescent + metrics.fLeading;
            }
        }
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ScaledEmojiRenderingGM;)
}
