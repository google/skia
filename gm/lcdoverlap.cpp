/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


/*
 * Tests overlapping LCD text
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTextBlob.h"
#include "tools/ToolUtils.h"

namespace skiagm {

constexpr int kWidth = 750;
constexpr int kHeight = 750;

class LcdOverlapGM : public skiagm::GM {
public:
    LcdOverlapGM() {
        const int kPointSize = 25;
        fTextHeight = SkIntToScalar(kPointSize);
    }

protected:
    SkString onShortName() override {
        return SkString("lcdoverlap");
    }

    void onOnceBeforeDraw() override {
        // build text blob
        SkTextBlobBuilder builder;

        SkFont      font(ToolUtils::create_portable_typeface(), 32);
        const char* text = "able was I ere I saw elba";
        font.setSubpixel(true);
        font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
        // If we use SkTextBlob::MakeFromText, we get very different positioning ... why?
        ToolUtils::add_to_text_blob(&builder, text, font, 0, 0);
        fBlob = builder.make();
    }

    SkISize onISize() override { return SkISize::Make(kWidth, kHeight); }

    void drawTestCase(SkCanvas* canvas, SkScalar x, SkScalar y, SkBlendMode mode,
                      SkBlendMode mode2) {
        const SkColor colors[] {
                SK_ColorRED,
                SK_ColorGREEN,
                SK_ColorBLUE,
                SK_ColorYELLOW,
                SK_ColorCYAN,
                SK_ColorMAGENTA,
        };

        for (size_t i = 0; i < SK_ARRAY_COUNT(colors); i++) {
            canvas->save();
            canvas->translate(x, y);
            canvas->rotate(360.0f / SK_ARRAY_COUNT(colors) * i);
            canvas->translate(-fBlob->bounds().width() / 2.0f + 0.5f, 0);

            SkPaint textPaint;
            textPaint.setColor(colors[i]);
            textPaint.setBlendMode(i % 2 == 0 ? mode : mode2);
            canvas->drawTextBlob(fBlob, 0, 0, textPaint);
            canvas->restore();
        }
    }

    void onDraw(SkCanvas* canvas) override {
        SkScalar offsetX = kWidth / 4.0f;
        SkScalar offsetY = kHeight / 4.0f;
        drawTestCase(canvas, offsetX, offsetY,  SkBlendMode::kSrc, SkBlendMode::kSrc);
        drawTestCase(canvas, 3 * offsetX, offsetY,  SkBlendMode::kSrcOver, SkBlendMode::kSrcOver);
        drawTestCase(canvas, offsetX, 3 * offsetY,  SkBlendMode::kHardLight,
                     SkBlendMode::kLuminosity);
        drawTestCase(canvas, 3 * offsetX, 3 * offsetY,  SkBlendMode::kSrcOver, SkBlendMode::kSrc);
    }

private:
    SkScalar fTextHeight;
    sk_sp<SkTextBlob> fBlob;
    typedef skiagm::GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new LcdOverlapGM; )
}
