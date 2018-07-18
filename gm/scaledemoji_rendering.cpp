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
    const char* fileNames[3] = { "fonts/colr.ttf", "fonts/sbix.ttf", "fonts/cbdt.ttf" };

    struct EmojiFont {
        sk_sp<SkTypeface> fTypeface;
        const char* fText;
    } fEmojiFont;

    void onOnceBeforeDraw() override {
        fEmojiFont.fTypeface = sk_tool_utils::emoji_typeface();
        fEmojiFont.fText = sk_tool_utils::emoji_sample_text();
    }

    sk_sp<SkTypeface> getEmojiTypeface(const char* filename) {
        sk_sp<SkTypeface> typeface = MakeResourceAsTypeface(filename);
        if (typeface) {
            return typeface;
        }
        return SkTypeface::MakeFromName("Emoji", SkFontStyle());
    }

    SkString onShortName() override {
        SkString name("scaledemoji_rendering");
        name.append(sk_tool_utils::platform_font_manager());
        return name;
    }

    SkISize onISize() override { return SkISize::Make(1200, 1200); }

    void onDraw(SkCanvas* canvas) override {

        canvas->drawColor(sk_tool_utils::color_to_565(SK_ColorGRAY));

        SkScalar y = 0;

        for (int f = 0; f < SK_ARRAY_COUNT(fileNames) + 1; f++) {
            SkPaint paint;
            if (f < SK_ARRAY_COUNT(fileNames)) {
                fEmojiFont.fTypeface = getEmojiTypeface(fileNames[f]);
                paint.setTypeface(fEmojiFont.fTypeface);
            }
            else {
                // Draw emoji by portable fontface
                fEmojiFont.fTypeface = sk_tool_utils::create_portable_typeface("Emoji",
                                                                               SkFontStyle());
                sk_tool_utils::set_portable_typeface(&paint, "Emoji", SkFontStyle());
            }
            fEmojiFont.fText = sk_tool_utils::emoji_sample_text();
            const char* text = fEmojiFont.fText;

            // draw text at different point sizes
            // Testing GPU bitmap path, SDF path with no scaling,
            // SDF path with scaling, path rendering with scaling
            SkPaint::FontMetrics metrics;

            for (SkScalar textSize : { 70, 150 /*, 270, 340 */}) {
                paint.setTextSize(textSize);
                paint.getFontMetrics(&metrics);
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