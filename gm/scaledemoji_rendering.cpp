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
#include "src/core/SkEnumerate.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

#include <string.h>
#include <initializer_list>

namespace skiagm {
class ScaledEmojiRenderingGM : public GM {
public:
    ScaledEmojiRenderingGM() {}

protected:
    struct Test {
        enum class Source { Resource, Portable };
        Source const fontSource;
        char const * const fontName;
        char const * const text;
    };
    static constexpr char const * const sampleText = ToolUtils::EmojiSampleText();
    static constexpr const Test tests[] = {
        { Test::Source::Resource, "fonts/colr.ttf"     , sampleText  },
        { Test::Source::Resource, "fonts/sbix.ttf"     , sampleText  },
        { Test::Source::Resource, "fonts/cbdt.ttf"     , sampleText  },
        { Test::Source::Portable, "Emoji"              , sampleText  },
        { Test::Source::Resource, "fonts/SampleSVG.ttf", "abcdefghij" },
    };
    sk_sp<SkTypeface> typefaces[std::size(tests)];
    void onOnceBeforeDraw() override {
        for (auto&& [i, test] : SkMakeEnumerate(tests)) {
            if (test.fontSource == Test::Source::Resource) {
                typefaces[i] = MakeResourceAsTypeface(test.fontName);
                if (!typefaces[i]) {
                    typefaces[i] = ToolUtils::DefaultTypeface();
                }
            } else if (test.fontSource == Test::Source::Portable) {
                typefaces[i] = ToolUtils::CreatePortableTypeface(test.fontName, SkFontStyle());
            } else {
                SK_ABORT("Unknown test type");
            }
        }
    }

    SkString getName() const override { return SkString("scaledemoji_rendering"); }

    SkISize getISize() override { return SkISize::Make(1200, 1200); }

    void onDraw(SkCanvas* canvas) override {

        canvas->drawColor(SK_ColorGRAY);
        SkPaint textPaint;
        textPaint.setColor(SK_ColorCYAN);

        SkPaint boundsPaint;
        boundsPaint.setStrokeWidth(2);
        boundsPaint.setStyle(SkPaint::kStroke_Style);
        boundsPaint.setColor(SK_ColorGREEN);

        SkPaint advancePaint;
        advancePaint.setColor(SK_ColorRED);

        SkScalar y = 0;
        for (auto&& [i, test] : SkMakeEnumerate(tests)) {
            SkFont font(typefaces[i]);
            font.setEdging(SkFont::Edging::kAlias);

            const char* text = test.text;
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
                    SkScalar advance = font.measureText(text, strlen(text), SkTextEncoding::kUTF8,
                                                        &bounds, &textPaint);
                    canvas->drawSimpleText(text, strlen(text), SkTextEncoding::kUTF8,
                                           x, y, font, textPaint);
                    if ((false)) {
                        bounds.offset(x, y);
                        canvas->drawRect(bounds, boundsPaint);
                        SkRect advanceRect = SkRect::MakeLTRB(x, y + 2, x + advance, y + 4);
                        canvas->drawRect(advanceRect, advancePaint);
                    }
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
