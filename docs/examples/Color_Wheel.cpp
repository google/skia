// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Color_Wheel, 256, 256, false, 0) {
void draw_color_wheel(SkCanvas* canvas, float scale) {
    const float stroke = scale * 3 / 64;
    const float size = scale * 9 / 32;
    SkAutoCanvasRestore autoCanvasRestore(canvas, true);
    canvas->translate(0.5f * scale, 0.5f * scale);
    canvas->clear(0);
    canvas->drawCircle({0, 0}, (scale - stroke) * 0.5f, SkPaint(SkColors::kWhite));
    {
        SkPaint sweep;
        const SkMatrix rotate = SkMatrix::MakeAll(0, -1, 0, 1, 0, 0, 0, 0, 1);
        constexpr unsigned kColorCount = 7;
        static const SkColor4f kColors[kColorCount] = {
            SkColors::kRed,  SkColors::kYellow,  SkColors::kGreen, SkColors::kCyan,
            SkColors::kBlue, SkColors::kMagenta, SkColors::kRed};
        sweep.setShader(SkGradientShader::MakeSweep(0, 0, kColors, nullptr, nullptr,
                                                    kColorCount, 0, &rotate));
        sweep.setStyle(SkPaint::kStroke_Style);
        sweep.setStrokeWidth(stroke);
        sweep.setAntiAlias(true);
        canvas->drawCircle({0, 0}, (scale - stroke) * 0.5f, sweep);
    }

    SkFont font(fontMgr->legacyMakeTypeface(nullptr, SkFontStyle::Bold()), size);
    const struct { const char* str; float radius; float angle; SkColor4f color;} kLetters[] = {
        {"K", 0,   0, SkColors::kBlack},
        {"R", 1,  90, SkColors::kRed},
        {"Y", 1, 150, SkColors::kYellow},
        {"G", 1, 210, SkColors::kGreen},
        {"C", 1, 270, SkColors::kCyan},
        {"B", 1, 330, SkColors::kBlue},
        {"M", 1,  30, SkColors::kMagenta},
    };
    for (const auto& v : kLetters) {
        SkRect bnds;
        font.measureText(v.str, strlen(v.str), SkTextEncoding::kUTF8, &bnds);
        constexpr double pi_over_180 = 3.141592653589793 / 180;
        float x = (float)(0.3 * scale * v.radius * cos(pi_over_180 * v.angle) - bnds.centerX());
        float y = (float)(0.3 * scale * v.radius * sin(pi_over_180 * v.angle) - bnds.centerY());
        canvas->drawString(v.str, x, y, font, SkPaint(v.color));
    }
}
void draw(SkCanvas* canvas) { draw_color_wheel(canvas, 256); }
}  // END FIDDLE
