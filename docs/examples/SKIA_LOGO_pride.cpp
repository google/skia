// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(SKIA_LOGO_pride, 816, 464, false, 0) {
void draw(SkCanvas* canvas) {
    canvas->scale(4.0f, 4.0f);
    constexpr SkColor background = SK_ColorWHITE;
    constexpr SkColor lettering = 0xFF292929;
    const SkColor4f lineColors[2] = {
            SkColor4f::FromColor(0x30565656), SkColor4f::FromColor(0xFF565656)};

    // Colors from https://www.flagcolorcodes.com/progress-pride and https://www.flagcolorcodes.com/intersex
    constexpr SkColor rainbow[] =  {0xFFE40303, 0xFFFF8C00, 0xFFFFED00, 0xFF008026, 0xFF004CFF, 0xFF732982};
    constexpr SkColor progress[] = {0xFFFFD800, 0xFFFFFFFF, 0xFFFFAFC8, 0xFF74D7EE, 0xFF613915, 0xFF000000};
    constexpr SkColor iViolet = 0xFF7902AA;

    SkPathBuilder s, k, a, triangle;
    SkPaint p;
    p.setAntiAlias(true);

    canvas->clear(background);
    canvas->scale(0.363f, 0.363f);

    float leftX = 310.f;
    constexpr float kIWidth = 50.f;
    constexpr float deltaWidth = kIWidth / std::size(rainbow);

    for (size_t i = 0; i < std::size(rainbow); ++i) {
        p.setColor(rainbow[i]);
        canvas->drawRect({leftX, 82.25f, leftX + deltaWidth, 249.2}, p);
        leftX += deltaWidth;
    }

    float triHeight = deltaWidth * (std::size(progress) + 1);

    canvas->save();
    canvas->clipRect({310.f, 82.25, 360.f, 249.2f}, SkClipOp::kIntersect, false);
    for (int i = std::size(progress)-1; i >= 0; --i) {
        p.setColor(progress[i]);
        SkPathBuilder t;
        t.moveTo(335.f, 82.25f + triHeight);
        t.rLineTo(200.f, -200.f);
        t.rLineTo(-400.f, 0.f);
        t.rLineTo(200.f, 200.f);
        canvas->drawPath(t.detach(), p);
        triHeight -= deltaWidth;
    }
    {
      SkPaint c;
      c.setColor(iViolet);
      c.setStroke(SkPaint::Style::kStroke_Style);
      c.setStrokeWidth(1.5f);
      c.setAntiAlias(true);

      canvas->drawCircle({335.f, 88.5f}, deltaWidth * 0.6f, c);
    }

    canvas->restore();

    p.setColor(lettering);
    canvas->drawCircle(335.355, 45.965, 29.25, p);

    s.moveTo(34.63, 100.63);
    s.cubicTo(44.38, 88.57, 59.87, 82.86, 74.88, 81.2);
    s.cubicTo(97.4, 78.5, 120.27, 83.25, 140.87, 92.37);
    s.lineTo(127.12, 127.14);
    s.cubicTo(113.55, 121.16, 99.04, 115.9, 83.98, 116.56);
    s.cubicTo(78.86, 116.75, 72.88, 118.54, 70.71, 123.69);
    s.cubicTo(68.62, 128.43, 71.52, 133.68, 75.58, 136.27);
    s.cubicTo(91.49, 146.66, 110.67, 151.38, 125.46, 163.6);
    s.cubicTo(132.35, 169.11, 137.33, 176.9, 139.36, 185.49);
    s.cubicTo(142.55, 199.14, 140.94, 214.31, 133.13, 226.17);
    s.cubicTo(126.23, 236.96, 114.82, 244.16, 102.75, 247.89);
    s.cubicTo(87.95, 252.51, 72.16, 252.21, 56.88, 250.78);
    s.cubicTo(45.54, 249.72, 34.64, 246.05, 24.32, 241.36);
    s.lineTo(24.25, 201.1);
    s.cubicTo(38.23, 208.15, 53.37, 213.15, 68.98, 214.75);
    s.cubicTo(75.42, 215.25, 82.17, 215.63, 88.31, 213.27);
    s.cubicTo(92.84, 211.53, 96.4, 206.93, 95.86, 201.93);
    s.cubicTo(95.64, 196.77, 91.1, 193.38, 87.03, 190.99);
    s.cubicTo(71.96, 182.67, 54.94, 177.66, 41.5, 166.57);
    s.cubicTo(33.19, 159.73, 27.51, 149.8, 26.1, 139.11);
    s.cubicTo(24.09, 125.88, 25.91, 111.25, 34.63, 100.63);
    canvas->drawPath(s.detach(), p);

    k.moveTo(160.82, 82.85);
    k.lineTo(206.05, 82.85);
    k.lineTo(206.05, 155.15);
    k.lineTo(254.83, 82.84);
    k.lineTo(304.01, 82.85);
    k.lineTo(251.52, 157.27);
    k.lineTo(303.09, 249.42);
    k.lineTo(252.28, 249.4);
    k.lineTo(219.18, 185.75);
    k.lineTo(206.23, 193.45);
    k.lineTo(206.05, 249.42);
    k.lineTo(160.82, 249.42);
    k.lineTo(160.82, 82.85);
    canvas->drawPath(k.detach(), p);

    a.moveTo(426.45, 218.16);
    a.lineTo(480.705, 218.16);
    a.lineTo(489.31, 249.4);
    a.lineTo(538.54, 249.42);
    a.lineTo(483.56, 82.18);
    a.lineTo(423.43, 82.17);
    a.lineTo(369.13, 249.42);
    a.lineTo(418.5, 249.47);
    a.lineTo(453.75, 109.83);
    a.lineTo(471.77, 181.28);
    a.lineTo(430.5, 181.28);
    canvas->drawPath(a.detach(), p);

    triangle.moveTo(362.64, 257.32);
    triangle.lineTo(335.292, 293.392);
    triangle.lineTo(307.8, 257.48);
    triangle.lineTo(362.64, 257.32);
    p.setColor(lettering);
    canvas->drawPath(triangle.detach(), p);

    constexpr SkPoint pts[2] = {{160, 290}, {341, 290}};
    p.setShader(SkShaders::LinearGradient(pts, {{lineColors, {}, SkTileMode::kClamp}, {}}));
    SkRRect rrect;
    rrect.setRectXY({138, 291, 341, 300}, 25.0, 5.0);
    canvas->drawRRect(rrect, p);
}
}  // END FIDDLE
