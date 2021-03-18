// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(picture_shader, 256, 256, false, 5) {
static void draw_centered(
        const char* s, const SkFont& font, SkColor color, SkPoint xy, SkCanvas* c) {
    sk_sp<SkTextBlob> b = SkTextBlob::MakeFromString(s, font);
    xy -= SkPoint{b->bounds().centerX(), b->bounds().centerY()};
    SkPaint p;
    p.setColor(color);
    c->drawTextBlob(b.get(), xy.x(), xy.y(), p);
}

SkPoint from_polar_deg(float r, float d) {
    float a = d * 0.017453292519943295;
    return {r * cosf(a), r * sinf(a)};
}

void draw_wheel(SkCanvas* c) {
    const SkScalar scale = 512;
    SkAutoCanvasRestore autoCanvasRestore(c, true);
    c->translate(0.5f * scale, 0.5f * scale);
    SkPaint p;
    p.setAntiAlias(true);
    p.setColor(SK_ColorWHITE);
    c->drawCircle(0.0f, 0.0f, scale * 0.475f, p);

    const SkColor sweep_colors[] = {SK_ColorRED,  SK_ColorYELLOW,  SK_ColorGREEN, SK_ColorCYAN,
                                    SK_ColorBLUE, SK_ColorMAGENTA, SK_ColorRED};
    SkMatrix rot;
    rot.setRotate(90.0f);
    p.setShader(SkGradientShader::MakeSweep(0, 0, sweep_colors, NULL,
                                            SK_ARRAY_COUNT(sweep_colors), 0, &rot));
    p.setStrokeWidth(0.05f * scale);
    p.setStyle(SkPaint::kStroke_Style);
    c->drawCircle(0.0f, 0.0f, 0.475f * scale, p);

    SkFont f(nullptr, 0.28125f * scale);
    draw_centered("K", f, SK_ColorBLACK, {0.0f, 0.0f}, c);
    draw_centered("R", f, SK_ColorRED, from_polar_deg(0.3f * scale, 90), c);
    draw_centered("G", f, SK_ColorGREEN, from_polar_deg(0.3f * scale, 210), c);
    draw_centered("B", f, SK_ColorBLUE, from_polar_deg(0.3f * scale, 330), c);
    draw_centered("C", f, SK_ColorCYAN, from_polar_deg(0.3f * scale, 270), c);
    draw_centered("M", f, SK_ColorMAGENTA, from_polar_deg(0.3f * scale, 30), c);
    draw_centered("Y", f, SK_ColorYELLOW, from_polar_deg(0.3f * scale, 150), c);
}

void draw(SkCanvas* canvas) {
    canvas->clear(SK_ColorWHITE);
    SkMatrix matrix;
    matrix.setScale(0.25f, 0.25f);
    matrix.preRotate(30.0f);
    SkPaint paint;
    SkPictureRecorder rec;
    draw_wheel(rec.beginRecording(512, 512));
    paint.setShader(rec.finishRecordingAsPicture()->makeShader(
            SkTileMode::kRepeat, SkTileMode::kRepeat, SkFilterMode::kNearest, &matrix, nullptr));
    canvas->drawPaint(paint);
}
}  // END FIDDLE
