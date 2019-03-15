#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=1c2e38321464818847f953ddd45cb5a1
REG_FIDDLE(Color_009, 256, 256, false, 0) {
#define SKIA_COLOR_PAIR(name) "SK_Color" #name, SK_Color##name

void draw(SkCanvas* canvas) {
    struct ColorCompare {
        const char* fSVGName;
        SkColor fSVGColor;
        const char* fSkiaName;
        SkColor fSkiaColor;
    } colorCompare[] = {  // see https://www.w3.org/TR/SVG/types.html#ColorKeywords
        {"black",     SkColorSetRGB(  0,   0,   0),    SKIA_COLOR_PAIR(BLACK) },
        {"darkgray",  SkColorSetRGB(169, 169, 169),    SKIA_COLOR_PAIR(DKGRAY) },
        {"gray",      SkColorSetRGB(128, 128, 128),    SKIA_COLOR_PAIR(GRAY) },
        {"lightgray", SkColorSetRGB(211, 211, 211),    SKIA_COLOR_PAIR(LTGRAY) },
        {"white",     SkColorSetRGB(255, 255, 255),    SKIA_COLOR_PAIR(WHITE) },
        {"red",       SkColorSetRGB(255,   0,   0),    SKIA_COLOR_PAIR(RED) },
        {"green",     SkColorSetRGB(  0, 128,   0),    SKIA_COLOR_PAIR(GREEN) },
        {"blue",      SkColorSetRGB(  0,   0, 255),    SKIA_COLOR_PAIR(BLUE) },
        {"yellow",    SkColorSetRGB(255, 255,   0),    SKIA_COLOR_PAIR(YELLOW) },
        {"aqua",      SkColorSetRGB(  0, 255, 255),    SKIA_COLOR_PAIR(CYAN) },
        {"fuchsia",   SkColorSetRGB(255,   0, 255),    SKIA_COLOR_PAIR(MAGENTA) },
    };
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setTextSize(14);
    for (auto compare : colorCompare) {
        paint.setStyle(SkPaint::kFill_Style);
        paint.setColor(compare.fSVGColor);
        canvas->drawRect({5, 5, 15, 15}, paint);
        paint.setColor(SK_ColorBLACK);
        canvas->drawString(compare.fSVGName, 20, 16, paint);
        paint.setColor(compare.fSkiaColor);
        canvas->drawRect({105, 5, 115, 15}, paint);
        paint.setColor(SK_ColorBLACK);
        canvas->drawString(compare.fSkiaName, 120, 16, paint);
        paint.setStyle(SkPaint::kStroke_Style);
        canvas->drawRect({5, 5, 15, 15}, paint);
        canvas->drawRect({105, 5, 115, 15}, paint);
        canvas->translate(0, 20);
    }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
