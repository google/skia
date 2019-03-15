#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=4e8b7409531c9211a2afcf632005a38c
REG_FIDDLE(Canvas_120, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    SkPoint cubics[] = { { 3, 1 },    { 4, 2 }, { 5, 1 },    { 7, 3 },
                      /* { 7, 3 }, */ { 6, 4 }, { 7, 5 },    { 5, 7 },
                      /* { 5, 7 }, */ { 4, 6 }, { 3, 7 },    { 1, 5 },
                      /* { 1, 5 }, */ { 2, 4 }, { 1, 3 }, /* { 3, 1 } */ };
    SkColor colors[] = { SK_ColorRED, SK_ColorBLUE, SK_ColorYELLOW, SK_ColorCYAN };
    canvas->scale(30, 30);
    canvas->drawPatch(cubics, colors, nullptr, paint);
    SkPoint text[] = { {3,0.9f}, {4,2.5f}, {5,0.9f}, {7.5f,3.2f}, {5.5f,4.2f},
            {7.5f,5.2f}, {5,7.5f}, {4,5.9f}, {3,7.5f}, {0.5f,5.2f}, {2.5f,4.2f},
            {0.5f,3.2f} };
    paint.setTextSize(18.f / 30);
    for (int i = 0; i< 10; ++i) {
       char digit = '0' + i;
       canvas->drawText(&digit, 1, text[i].fX, text[i].fY, paint);
    }
    canvas->drawString("10", text[10].fX, text[10].fY, paint);
    canvas->drawString("11", text[11].fX, text[11].fY, paint);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawPoints(SkCanvas::kPolygon_PointMode, 12, cubics, paint);
    canvas->drawLine(cubics[11].fX, cubics[11].fY, cubics[0].fX, cubics[0].fY, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
