// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(SKIA_LOGO_svg, 816, 464, false, 0) {
void draw(SkCanvas* canvas) {
    canvas->scale(4.0f, 4.0f);
    const SkColor background = SK_ColorWHITE;  // SK_ColorTRANSPARENT;
    const SkColor rgb[] = {0xFFE94037, 0xFF70BF4F, 0xFF465BA6};
    const SkColor lettering = 0xFF292929;
    const SkColor lineColors[2] = {0x30565656, 0xFF565656};
    SkPath s, k, a, triangle;
    SkPaint p;
    p.setAntiAlias(true);

    canvas->clear(background);
    canvas->scale(0.363f, 0.363f);

    p.setColor(rgb[1]);
    canvas->drawRect({326.0, 82.25, 343.9, 249.2}, p);
    p.setColor(rgb[0]);
    canvas->drawRect({310.2, 82.25, 327.0, 249.2}, p);
    p.setColor(rgb[2]);
    canvas->drawRect({342.9, 82.25, 358.87, 249.2}, p);

    p.setColor(lettering);
    canvas->drawCircle(335.355, 45.965, 29.25, p);

    SkParsePath::FromSVGString("M34.63 100.63C44.38 88.57 59.87 82.86 74.88 81.2"
    "C97.4 78.5 120.27 83.25 140.87 92.37L127.12 127.14C113.55 121.16 99.04 115.9 83.98 116.56"
    "C78.86 116.75 72.88 118.54 70.71 123.69C68.62 128.43 71.52 133.68 75.58 136.27"
    "C91.49 146.66 110.67 151.38 125.46 163.6C132.35 169.11 137.33 176.9 139.36 185.49"
    "C142.55 199.14 140.94 214.31 133.13 226.17C126.23 236.96 114.82 244.16 102.75 247.89"
    "C87.95 252.51 72.16 252.21 56.88 250.78C45.54 249.72 34.64 246.05 24.32 241.36"
    "L24.25 201.1C38.23 208.15 53.37 213.15 68.98 214.75C75.42 215.25 82.17 215.63 88.31 213.27"
    "C92.84 211.53 96.4 206.93 95.86 201.93C95.64 196.77 91.1 193.38 87.03 190.99"
    "C71.96 182.67 54.94 177.66 41.5 166.57C33.19 159.73 27.51 149.8 26.1 139.11"
    "C24.09 125.88 25.91 111.25 34.63 100.63", &s);
    canvas->drawPath(s, p);

    SkParsePath::FromSVGString("M160.82 82.85L206.05 82.85L206.05 155.15L254.83 82.84"
    "L304.01 82.85L251.52 157.27L303.09 249.42L252.28 249.4L219.18 185.75L206.23 193.45"
    "L206.05 249.42L160.82 249.42L160.82 82.85", &k);
    canvas->drawPath(k, p);

    SkParsePath::FromSVGString("M426.45 218.16L480.705 218.16L489.31 249.4L538.54 249.42"
    "L483.56 82.18L423.43 82.17L369.13 249.42L418.5 249.47L453.75 109.83L471.77 181.28"
    "L430.5 181.28", &a);
    canvas->drawPath(a, p);

    SkParsePath::FromSVGString("M362.64 257.32L335.292 293.392L307.8 257.48L362.64 257.32",
    &triangle);
    canvas->drawPath(triangle, p);

    const SkPoint pts[2] = {{160, 290}, {341, 290}};
    p.setShader(SkGradientShader::MakeLinear(
            pts, lineColors, NULL, 2, SkTileMode::kClamp));
    SkRRect rrect;
    rrect.setRectXY({138, 291, 341, 300}, 25.0, 5.0);
    canvas->drawRRect(rrect, p);
}
}  // END FIDDLE
