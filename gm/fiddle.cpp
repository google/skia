/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "skia.h"

static void draw(SkCanvas* canvas);
DEF_SIMPLE_GM(fiddle, canvas, 256, 256) { draw(canvas); }

void drawOne(SkCanvas* canvas,
             double x,
             double y,
             double w,
             double h,
             double border,
             double sigma,
             SkColor c) {
    SkPaint sp;
    sp.setImageFilter(SkImageFilters::Blur(sigma, sigma, SkTileMode::kClamp, nullptr));
    SkRect r = SkRect::MakeXYWH(x, y, w, h);
    SkPaint p;
    p.setColor(c);
    p.setAntiAlias(true);

    canvas->saveLayer(r.makeOutset(border, border), &sp);
    canvas->drawRect(r, p);
    canvas->restore();
}

#if 0
void draw(SkCanvas* canvas) {
    drawOne(canvas, 64, 64, 25, 25, 1, 3, SK_ColorGREEN);
    drawOne(canvas, 164, 64, 25, 25, 30, 3, SK_ColorGREEN);
    drawOne(canvas, 64, 164, 25, 25, 1, 20, SK_ColorRED);
    drawOne(canvas, 164, 164, 25, 25, 30, 20, SK_ColorGREEN);
}
#else
#include "tools/ToolUtils.h"
void draw(SkCanvas* canvas) {
    auto image = ToolUtils::create_checkerboard_image(64, 64, SK_ColorBLACK, SK_ColorWHITE, 16);
    SkPaint p;
    p.setColor(SK_ColorYELLOW);
    canvas->drawRect(SkRect::MakeXYWH(0, 0, 256, 256), p);
    canvas->drawImage(image, 10, 10, nullptr);

    SkPaint p2;
    p2.setImageFilter(SkImageFilters::Blur(2.0, 2.0, SkTileMode::kClamp, nullptr, nullptr));
    canvas->drawImage(image, 110, 10, &p2);
    canvas->drawImage(image, 10.5, 110.5, &p2);
    canvas->translate(0.5, 0.5);
    canvas->drawImage(image, 110, 110, &p2);
}
#endif