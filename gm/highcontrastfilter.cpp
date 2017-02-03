/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkHighContrastFilter.h"

static SkScalar kSize   = 460;
static SkColor  kColor1 = SkColorSetARGB(0xff, 0xff, 0xff, 0);
static SkColor  kColor2 = SkColorSetARGB(0xff, 0x82, 0xff, 0);

static void draw_label(SkCanvas* canvas, const SkHighContrastConfig& config) {
    char labelBuffer[256];
    const char* invertStr =
        (config.fInvertStyle == HighContrastInvertStyle::kInvertBrightness ?
             "InvBrightness" :
            (config.fInvertStyle == HighContrastInvertStyle::kInvertLightness ?
                 "InvLightness" : "NoInvert"));

    snprintf(labelBuffer, sizeof(labelBuffer), "%s%s exp=%.1f cnt=%.1f",
             config.fGrayscale ? "Gray " : "",
             invertStr,
             config.fExponent,
             config.fContrast);

    SkPaint paint;
    sk_tool_utils::set_portable_typeface(&paint);
    paint.setTextSize(0.05f);
    size_t len = strlen(labelBuffer);

    SkScalar width = paint.measureText(labelBuffer, len);
    canvas->drawText(labelBuffer, len, 0.5f - width / 2, 0.16f, paint);
}

static void draw_scene(SkCanvas* canvas, const SkHighContrastConfig& config) {
    SkRect bounds = SkRect::MakeLTRB(0.0f, 0.0f, 1.0f, 1.0f);
    SkPaint xferPaint;
    xferPaint.setColorFilter(SkHighContrastFilter::Make(config));
    canvas->saveLayer(&bounds, &xferPaint);

    SkPaint paint;
    bounds = SkRect::MakeLTRB(0.1f, 0.2f, 0.9f, 0.4f);
    paint.setARGB(0xff, 0x66, 0x11, 0x11);
    canvas->drawRect(bounds, paint);

    paint.setARGB(0xff, 0xaa, 0x66, 0x66);
    paint.setTextSize(0.15f);
    canvas->drawText("A", 1, 0.15f, 0.35f, paint);

    bounds = SkRect::MakeLTRB(0.1f, 0.8f, 0.9f, 1.0f);
    paint.setARGB(0xff, 0xcc, 0xcc, 0xff);
    canvas->drawRect(bounds, paint);

    paint.setARGB(0xff, 0x88, 0x88, 0xbb);
    paint.setTextSize(0.15f);
    canvas->drawText("Z", 1, 0.75f, 0.95f, paint);

    bounds = SkRect::MakeLTRB(0.1f, 0.4f, 0.9f, 0.6f);
    SkPoint     pts[] = { { 0, 0 }, { 1, 0 } };
    SkColor     colors[] = { SK_ColorWHITE, SK_ColorBLACK };
    SkScalar    pos[] = { 0.2f, 0.8f };
    paint.setShader(SkGradientShader::MakeLinear(
        pts, colors, pos,
        SK_ARRAY_COUNT(colors), SkShader::kClamp_TileMode));
    canvas->drawRect(bounds, paint);

    bounds = SkRect::MakeLTRB(0.1f, 0.6f, 0.9f, 0.8f);
    SkColor colors2[] = { SK_ColorGREEN, SK_ColorWHITE };
    paint.setShader(SkGradientShader::MakeLinear(
        pts, colors2, pos,
        SK_ARRAY_COUNT(colors2), SkShader::kClamp_TileMode));
    canvas->drawRect(bounds, paint);

    canvas->restore();
}

class HighContrastFilterGM : public skiagm::GM {
public:
    HighContrastFilterGM() {
        SkColor  g1Colors[] = { kColor1, SkColorSetA(kColor1, 0x20) };
        SkColor  g2Colors[] = { kColor2, SkColorSetA(kColor2, 0x20) };
        SkPoint  g1Points[] = { { 0, 0 }, { 0,     100 } };
        SkPoint  g2Points[] = { { 0, 0 }, { kSize, 0   } };
        SkScalar pos[] = { 0.2f, 1.0f };

        SkHighContrastConfig fConfig;
        fFilter = SkHighContrastFilter::Make(fConfig);
        fGr1 = SkGradientShader::MakeLinear(
            g1Points, g1Colors, pos, SK_ARRAY_COUNT(g1Colors),
            SkShader::kClamp_TileMode);
        fGr2 = SkGradientShader::MakeLinear(
            g2Points, g2Colors, pos, SK_ARRAY_COUNT(g2Colors),
            SkShader::kClamp_TileMode);
    }

protected:

    SkString onShortName() override {
        return SkString("highcontrastfilter");
    }

    SkISize onISize() override {
        return SkISize::Make(600, 420);
    }

    void onDraw(SkCanvas* canvas) override {
        SkHighContrastConfig configs[] = {
            { false, HighContrastInvertStyle::kNoInvert, 1.0f, 0.0f },
            { false, HighContrastInvertStyle::kInvertBrightness, 1.0f, 0.0f },
            { false, HighContrastInvertStyle::kInvertLightness, 1.0f, 0.0f },
            { false, HighContrastInvertStyle::kInvertLightness, 1.0f, 0.3f },
            { false, HighContrastInvertStyle::kNoInvert, 0.5f, 0.0f },
            { false, HighContrastInvertStyle::kInvertBrightness, 0.5f, 0.0f },
            { false, HighContrastInvertStyle::kInvertLightness, 0.5f, 0.0f },
            { false, HighContrastInvertStyle::kInvertLightness, 0.5f, 0.3f },
            { true, HighContrastInvertStyle::kNoInvert, 1.0f, 0.0f },
            { true, HighContrastInvertStyle::kInvertBrightness, 1.0f, 0.0f },
            { true, HighContrastInvertStyle::kInvertLightness, 1.0f, 0.0f },
            { true, HighContrastInvertStyle::kInvertLightness, 1.0f, 0.3f },
            { true, HighContrastInvertStyle::kNoInvert, 0.5f, 0.0f },
            { true, HighContrastInvertStyle::kInvertBrightness, 0.5f, 0.0f },
            { true, HighContrastInvertStyle::kInvertLightness, 0.5f, 0.0f },
            { true, HighContrastInvertStyle::kInvertLightness, 0.5f, 0.3f },
        };

        for (size_t i = 0; i < SK_ARRAY_COUNT(configs); ++i) {
            SkScalar x = kSize * (i % 4);
            SkScalar y = kSize * (i / 4);
            canvas->save();
            canvas->translate(x, y);
            canvas->scale(kSize, kSize);
            draw_scene(canvas, configs[i]);
            draw_label(canvas, configs[i]);
            canvas->restore();
        }
    }

private:
    sk_sp<SkColorFilter>    fFilter;
    sk_sp<SkShader>         fGr1, fGr2;

    typedef skiagm::GM INHERITED;
};

DEF_GM(return new HighContrastFilterGM;)
