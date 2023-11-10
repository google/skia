/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

/**
 *  Skia may draw from outlines when the size is very large, so we exercise that
 *  here.
*/

class BigTextGM : public skiagm::GM {
public:
    BigTextGM() {}

protected:
    SkString getName() const override { return SkString("bigtext"); }

    SkISize getISize() override { return SkISize::Make(640, 480); }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setAntiAlias(true);
        SkFont font(ToolUtils::DefaultPortableTypeface(), 1500);

        SkRect r;
        (void)font.measureText("/", 1, SkTextEncoding::kUTF8, &r);
        SkPoint pos = {
            this->width()/2 - r.centerX(),
            this->height()/2 - r.centerY()
        };

        paint.setColor(SK_ColorRED);
        canvas->drawSimpleText("/", 1, SkTextEncoding::kUTF8, pos.fX, pos.fY, font, paint);

        paint.setColor(SK_ColorBLUE);
        canvas->drawSimpleText("\\", 1, SkTextEncoding::kUTF8, pos.fX, pos.fY, font, paint);
    }

private:
    using INHERITED = skiagm::GM;
};

DEF_GM(return new BigTextGM;)

// Exercise the case where the glyph is sufficiently large that we should just draw with a path,
// but the DirectWrite scaler context failed to calculate the bounds and reported empty bounds.
// With empty bounds the glyph was discarded instead of rendered from path. See crbug.com/1370488
DEF_SIMPLE_GM(bigtext_crbug_1370488, canvas, 512, 512) {
    auto typeface = ToolUtils::CreateTypefaceFromResource("fonts/SpiderSymbol.ttf");
    const char* text = "\xEF\x80\xA1";
    if (!typeface) {
        typeface = ToolUtils::DefaultPortableTypeface();
        text = "H";
    }

    SkFont font(typeface, 12.f);
    canvas->translate(-1800.f, 1800.f);
    canvas->scale(437.5f, 437.5f);
    SkPaint paint;
    paint.setAntiAlias(true);
    canvas->drawString(text, 0.f, 0.f, font, paint);
}
