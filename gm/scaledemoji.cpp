/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"

#include "Resources.h"
#include "SkCanvas.h"
#include "SkStream.h"
#include "SkTypeface.h"

namespace skiagm {

class ScaledEmojiGM : public GM {
public:
    ScaledEmojiGM() { }

protected:
    struct EmojiFont {
        sk_sp<SkTypeface> typeface;
        const char* text;
    } emojiFont;
    virtual void onOnceBeforeDraw() override {
        emojiFont.typeface = sk_tool_utils::emoji_typeface();
        emojiFont.text = sk_tool_utils::emoji_sample_text();
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
        paint.setTypeface(emojiFont.typeface);
        const char* text = emojiFont.text;

        // draw text at different point sizes
        // Testing GPU bitmap path, SDF path with no scaling,
        // SDF path with scaling, path rendering with scaling
        constexpr SkScalar textSizes[] = { 70, 180, 270, 340 };
        SkPaint::FontMetrics metrics;
        SkScalar y = 0;
        for (const SkScalar& textSize : textSizes) {
            paint.setTextSize(textSize);
            paint.getFontMetrics(&metrics);
            y += -metrics.fAscent;
            canvas->drawString(text, 10, y, paint);
            y += metrics.fDescent + metrics.fLeading;
        }

    }

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ScaledEmojiGM;)

}
