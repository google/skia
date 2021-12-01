/*
* Copyright 2018 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

#include <string.h>
#include <initializer_list>

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
        typefaces[3] = ToolUtils::create_portable_typeface("Emoji", SkFontStyle());
    }

    SkString onShortName() override {
        return SkString("scaledemoji_rendering");
    }

    SkISize onISize() override { return SkISize::Make(1200, 1200); }

    void onDraw(SkCanvas* canvas) override {

        canvas->drawColor(SK_ColorGRAY);
        SkPaint paint;
        paint.setColor(SK_ColorCYAN);

        SkScalar y = 0;
        for (const auto& typeface: typefaces) {
            SkFont font(typeface);
            font.setEdging(SkFont::Edging::kAlias);

            const char*   text = ToolUtils::emoji_sample_text();
            SkFontMetrics metrics;

            for (SkScalar textSize : { 70, 150 }) {
                font.setSize(textSize);
                font.getMetrics(&metrics);
                // All typefaces should support subpixel mode
                font.setSubpixel(true);

                y += -metrics.fAscent;

                SkScalar x = 0;
                for (bool fakeBold : { false, true }) {
                    font.setEmbolden(fakeBold);
                    SkRect bounds;
                    font.measureText(text, strlen(text), SkTextEncoding::kUTF8, &bounds, &paint);
                    canvas->drawSimpleText(text, strlen(text), SkTextEncoding::kUTF8,
                                           x + bounds.left(), y, font, paint);
                    x += bounds.width() * 1.2;
                }
                y += metrics.fDescent + metrics.fLeading;
                x = 0;
            }
        }
    }

private:
    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ScaledEmojiRenderingGM;)
}  // namespace skiagm
