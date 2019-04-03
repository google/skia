/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkPath.h"
#include "SkTypeface.h"
#include "ToolUtils.h"
#include "gm.h"

static sk_sp<SkShader> make_heatGradient(const SkPoint pts[2]) {
    const SkColor bw[] = { SK_ColorBLACK, SK_ColorWHITE };

    return SkGradientShader::MakeLinear(pts, bw, nullptr, SK_ARRAY_COUNT(bw), SkTileMode::kClamp);
}

/**
   Test a set of clipping problems discovered while writing blitAntiRect,
   and test all the code paths through the clipping blitters.
   Each region should show as a blue center surrounded by a 2px green
   border, with no red.
*/

#define HEIGHT 480

class GammaTextGM : public skiagm::GM {
protected:
    SkString onShortName() override {
        return SkString("gammatext");
    }

    SkISize onISize() override {
        return SkISize::Make(1024, HEIGHT);
    }

    static void drawGrad(SkCanvas* canvas) {
        const SkPoint pts[] = { { 0, 0 }, { 0, SkIntToScalar(HEIGHT) } };

        canvas->clear(SK_ColorRED);
        SkPaint paint;
        paint.setShader(make_heatGradient(pts));
        SkRect r = { 0, 0, SkIntToScalar(1024), SkIntToScalar(HEIGHT) };
        canvas->drawRect(r, paint);
    }

    void onDraw(SkCanvas* canvas) override {
        drawGrad(canvas);

        const SkColor fg[] = {
            0xFFFFFFFF,
            0xFFFFFF00, 0xFFFF00FF, 0xFF00FFFF,
            0xFFFF0000, 0xFF00FF00, 0xFF0000FF,
            0xFF000000,
        };

        const char* text = "Hamburgefons";

        SkPaint paint;
        SkFont font(nullptr, 16);
        font.setEdging(SkFont::Edging::kSubpixelAntiAlias);

        SkScalar x = SkIntToScalar(10);
        for (size_t i = 0; i < SK_ARRAY_COUNT(fg); ++i) {
            paint.setColor(fg[i]);

            SkScalar y = SkIntToScalar(40);
            SkScalar stopy = SkIntToScalar(HEIGHT);
            while (y < stopy) {
                canvas->drawString(text, x, y, font, paint);
                y += font.getSize() * 2;
            }
            x += SkIntToScalar(1024) / SK_ARRAY_COUNT(fg);
        }
    }

private:
    typedef skiagm::GM INHERITED;
};

DEF_GM( return new GammaTextGM; )

//////////////////////////////////////////////////////////////////////////////

static sk_sp<SkShader> make_gradient(SkColor c) {
    const SkPoint pts[] = { { 0, 0 }, { 240, 0 } };
    SkColor colors[2];
    colors[0] = c;
    colors[1] = SkColorSetA(c, 0);
    return SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kClamp);
}

static void draw_pair(SkCanvas* canvas, const SkFont& font, SkColor color,
                      const sk_sp<SkShader>& shader) {
    static const char text[] = "Now is the time for all good";
    SkPaint paint;
    paint.setColor(color);
    canvas->drawString(text, 10, 20, font, paint);
    paint.setShader(SkShader::MakeColorShader(paint.getColor()));
    canvas->drawString(text, 10, 40, font, paint);
    paint.setShader(shader);
    canvas->drawString(text, 10, 60, font, paint);
}

class GammaShaderTextGM : public skiagm::GM {
    sk_sp<SkShader> fShaders[3];
    SkColor fColors[3];

public:
    GammaShaderTextGM() {
        const SkColor colors[] = { SK_ColorBLACK, SK_ColorRED, SK_ColorBLUE };
        for (size_t i = 0; i < SK_ARRAY_COUNT(fShaders); ++i) {
            fColors[i] = colors[i];
        }
    }

protected:
    SkString onShortName() override {
        return SkString("gammagradienttext");
    }

    SkISize onISize() override {
        return SkISize::Make(300, 300);
    }

    void onOnceBeforeDraw() override {
        for (size_t i = 0; i < SK_ARRAY_COUNT(fShaders); ++i) {
            fShaders[i] = make_gradient(fColors[i]);
        }
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setAntiAlias(true);
        SkFont font(SkTypeface::MakeFromName("serif", SkFontStyle::Italic()), 18);
        font.setEdging(SkFont::Edging::kSubpixelAntiAlias);

        for (size_t i = 0; i < SK_ARRAY_COUNT(fShaders); ++i) {
            draw_pair(canvas, font, fColors[i], fShaders[i]);
            canvas->translate(0, 80);
        }
    }

private:
    typedef skiagm::GM INHERITED;
};

DEF_GM( return new GammaShaderTextGM; )
