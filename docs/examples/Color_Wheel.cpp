// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Color_Wheel, 256, 256, false, 0) {
static void draw_center_letter(SkCanvas* canvas, const char* str , const SkFont& font,
                               SkScalar x, SkScalar y, SkColor4f color) {
    SkRect bnds;
    font.measureText(str, strlen(str), SkTextEncoding::kUTF8, &bnds);
    canvas->drawString(str, x - bnds.centerX(), y - bnds.centerY(), font, SkPaint(color));
}

void draw(SkCanvas* canvas) {
     constexpr float SCALE = 256;
     constexpr float STROKE = 12;
     SkAutoCanvasRestore autoCanvasRestore(canvas, true);
     canvas->translate(0.5f * SCALE, 0.5f * SCALE);
     canvas->clear(0);
     canvas->drawCircle({0, 0}, (SCALE - STROKE) * 0.5f, SkPaint(SkColors::kWhite));
     {
         SkPaint sweep;
         const SkMatrix rotate = SkMatrix::MakeAll(0, -1, 0, 1, 0, 0, 0, 0, 1);
         static const SkColor4f kColors[7] = {
             SkColors::kRed,  SkColors::kYellow,  SkColors::kGreen, SkColors::kCyan,
             SkColors::kBlue, SkColors::kMagenta, SkColors::kRed};
         sweep.setShader(
                 SkGradientShader::MakeSweep(0, 0, kColors, nullptr, nullptr, 7, 0, &rotate));
         sweep.setStyle(SkPaint::kStroke_Style);
         sweep.setStrokeWidth(STROKE);
         canvas->drawCircle({0, 0}, (SCALE - STROKE) * 0.5f, sweep);
     }
     const double sqrt_3_over_2 = 0.8660254037844387;
     const SkScalar D = 0.3f * SCALE;
     const SkScalar X = (float)(D * sqrt_3_over_2);
     const SkScalar Y = D * 0.5f;

     sk_sp<SkFontMgr> fontMgr = SkFontMgr::RefDefault();
     SkFont font(fontMgr->legacyMakeTypeface(nullptr, SkFontStyle::Bold()), 0.28125f * SCALE);
     draw_center_letter(canvas, "K", font,  0,  0, SkColors::kBlack);
     draw_center_letter(canvas, "R", font,  0,  D, SkColors::kRed);
     draw_center_letter(canvas, "G", font, -X, -Y, SkColors::kGreen);
     draw_center_letter(canvas, "B", font,  X, -Y, SkColors::kBlue);
     draw_center_letter(canvas, "C", font,  0, -D, SkColors::kCyan);
     draw_center_letter(canvas, "M", font,  X,  Y, SkColors::kMagenta);
     draw_center_letter(canvas, "Y", font, -X,  Y, SkColors::kYellow);
}
}  // END FIDDLE
