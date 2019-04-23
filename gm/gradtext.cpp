/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkTypeface.h"
#include "include/effects/SkGradientShader.h"
#include "tools/ToolUtils.h"

// test shader w/ transparency
static sk_sp<SkShader> make_grad(SkScalar width) {
    SkColor colors[] = { SK_ColorRED, 0x0000FF00, SK_ColorBLUE };
    SkPoint pts[] = { { 0, 0 }, { width, 0 } };
    return SkGradientShader::MakeLinear(pts, colors, nullptr, SK_ARRAY_COUNT(colors),
                                        SkTileMode::kMirror);
}

// test opaque shader
static sk_sp<SkShader> make_grad2(SkScalar width) {
    SkColor colors[] = { SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE };
    SkPoint pts[] = { { 0, 0 }, { width, 0 } };
    return SkGradientShader::MakeLinear(pts, colors, nullptr, SK_ARRAY_COUNT(colors),
                                        SkTileMode::kMirror);
}

static sk_sp<SkShader> make_chrome_solid() {
    SkColor colors[] = { SK_ColorGREEN, SK_ColorGREEN };
    SkPoint pts[] = { { 0, 0 }, { 1, 0 } };
    return SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kClamp);
}

namespace skiagm {

// Replicate chrome layout test - clipped pathed gradient-shaded text
class ChromeGradTextGM1 : public GM {
public:
    ChromeGradTextGM1() { }
protected:

    virtual SkString onShortName() { return SkString("chrome_gradtext1"); }
    virtual SkISize onISize() { return SkISize::Make(500, 480); }
    virtual void onDraw(SkCanvas* canvas) {
        SkPaint paint;
        SkRect r = SkRect::MakeWH(SkIntToScalar(100), SkIntToScalar(100));

        canvas->clipRect(r);

        paint.setColor(SK_ColorRED);
        canvas->drawRect(r, paint);

        // Minimal repro doesn't require AA, LCD, or a nondefault typeface
        paint.setShader(make_chrome_solid());

        SkFont font(ToolUtils::create_portable_typeface(), 500);
        font.setEdging(SkFont::Edging::kAlias);

        canvas->drawString("I", 0, 100, font, paint);
    }
private:
    typedef GM INHERITED;
};


// Replicate chrome layout test - switching between solid & gradient text
class ChromeGradTextGM2 : public GM {
public:
    ChromeGradTextGM2() { }
protected:

    virtual SkString onShortName() { return SkString("chrome_gradtext2"); }
    virtual SkISize onISize() { return SkISize::Make(500, 480); }
    virtual void onDraw(SkCanvas* canvas) {
        SkPaint paint;
        SkFont  font(ToolUtils::create_portable_typeface());
        font.setEdging(SkFont::Edging::kAlias);

        paint.setStyle(SkPaint::kFill_Style);
        canvas->drawString("Normal Fill Text", 0, 50, font, paint);
        paint.setStyle(SkPaint::kStroke_Style);
        canvas->drawString("Normal Stroke Text", 0, 100, font, paint);

        // Minimal repro doesn't require AA, LCD, or a nondefault typeface
        paint.setShader(make_chrome_solid());

        paint.setStyle(SkPaint::kFill_Style);
        canvas->drawString("Gradient Fill Text", 0, 150, font, paint);
        paint.setStyle(SkPaint::kStroke_Style);
        canvas->drawString("Gradient Stroke Text", 0, 200, font, paint);
    }
private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new ChromeGradTextGM1; )
DEF_GM( return new ChromeGradTextGM2; )
}

DEF_SIMPLE_GM(gradtext, canvas, 500, 480) {
    static constexpr float kTextSize = 26.0f;
    SkFont                 font(ToolUtils::create_portable_typeface(), kTextSize);

    canvas->drawRect({0, 0, 500, 240}, SkPaint());
    canvas->translate(20.0f, kTextSize);

    SkPaint paints[2];
    paints[0].setShader(make_grad(80.0f));
    paints[1].setShader(make_grad2(80.0f));

    static const SkFont::Edging edgings[3] = {
        SkFont::Edging::kAlias,
        SkFont::Edging::kAntiAlias,
        SkFont::Edging::kSubpixelAntiAlias,
    };
    for (int i = 0; i < 2; ++i) {
        for (const SkPaint& paint : paints) {
            for (SkFont::Edging edging : edgings) {
                font.setEdging(edging);
                canvas->drawString("When in the course of human events", 0, 0, font, paint);
                canvas->translate(0, kTextSize * 4/3);
            }
            canvas->translate(0, kTextSize * 2/3);
        }
    }
}
