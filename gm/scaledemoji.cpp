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

class ScaledEmojiGM : public GM {
public:
    ScaledEmojiGM() { }

protected:
    struct EmojiFont {
        sk_sp<SkTypeface> fTypeface;
        const char* fText;
    } fEmojiFont;

    void onOnceBeforeDraw() override {
        fEmojiFont.fTypeface = sk_tool_utils::emoji_typeface();
        fEmojiFont.fText = sk_tool_utils::emoji_sample_text();
    }

    SkString onShortName() override {
        SkString name("scaledemoji");
        name.append(sk_tool_utils::platform_font_manager());
        return name;
    }

    SkISize onISize() override { return SkISize::Make(1200, 1200); }

    void onDraw(SkCanvas* canvas) override {

        canvas->drawColor(sk_tool_utils::color_to_565(SK_ColorGRAY));

        SkPaint paint;
        paint.setTypeface(fEmojiFont.fTypeface);
        const char* text = fEmojiFont.fText;

        // draw text at different point sizes
        // Testing GPU bitmap path, SDF path with no scaling,
        // SDF path with scaling, path rendering with scaling
        SkPaint::FontMetrics metrics;
        SkScalar y = 0;
        for (SkScalar textSize : { 70, 180, 270, 340 }) {
            paint.setTextSize(textSize);
            paint.getFontMetrics(&metrics);
            y += -metrics.fAscent;
            canvas->drawString(text, 10, y, paint);
            y += metrics.fDescent + metrics.fLeading;
        }

    }

private:
    typedef GM INHERITED;
};

class ScaledEmojiPosGM : public GM {
public:
    ScaledEmojiPosGM() {}

protected:
    struct EmojiFont {
        sk_sp<SkTypeface> fTypeface;
        const char* fText;
    } fEmojiFont;

    void onOnceBeforeDraw() override {
        fEmojiFont.fTypeface = sk_tool_utils::emoji_typeface();
        fEmojiFont.fText = sk_tool_utils::emoji_sample_text();
    }

    SkString onShortName() override {
        SkString name("scaledemojipos");
        name.append(sk_tool_utils::platform_font_manager());
        return name;
    }

    SkISize onISize() override { return SkISize::Make(1200, 1200); }

    void onDraw(SkCanvas* canvas) override {

        canvas->drawColor(sk_tool_utils::color_to_565(SK_ColorGRAY));

        SkPaint paint;
        paint.setTypeface(fEmojiFont.fTypeface);
        const char* text = fEmojiFont.fText;

        // draw text at different point sizes
        // Testing GPU bitmap path, SDF path with no scaling,
        // SDF path with scaling, path rendering with scaling
        SkPaint::FontMetrics metrics;
        SkScalar y = 0;
        for (SkScalar textSize : { 70, 180, 270, 340 }) {
            paint.setTextSize(textSize);
            paint.getFontMetrics(&metrics);
            y += -metrics.fAscent;

            int len = SkToInt(strlen(text));
            SkAutoTArray<SkPoint>  pos(len);
            SkAutoTArray<SkScalar> widths(len);
            paint.getTextWidths(text, len, &widths[0]);

            SkScalar x = SkIntToScalar(10);
            for (int i = 0; i < len; ++i) {
                pos[i].set(x, y);
                x += widths[i];
            }

            canvas->drawPosText(text, len, &pos[0], paint);
            y += metrics.fDescent + metrics.fLeading;
        }

    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ScaledEmojiGM;)
DEF_GM(return new ScaledEmojiPosGM;)

}
