#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=a4233634c75b72fc7a2815ddb69bd669
REG_FIDDLE(RRect_Type, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    struct Radii { SkVector data[4]; };
    auto drawRRectType = [=](const SkRect& rect, const Radii& radii) {
        SkRRect rrect;
        rrect.setRectRadii(rect, radii.data);
        SkPaint paint;
        paint.setAntiAlias(true);
        const char* typeStr[] = { "empty", "rect", "oval", "simple", "nine patch", "complex" };
        canvas->drawString(typeStr[(int) rrect.type()], rect.centerX(), rect.bottom() + 20, paint);
        paint.setStyle(SkPaint::kStroke_Style);
        canvas->drawRRect(rrect, paint);
    };
    drawRRectType({ 45,  30,  45,  30}, {{{ 5,  5}, { 5,  5}, { 5,  5}, { 5,  5}}});
    drawRRectType({ 90,  10, 140,  30}, {{{ 0,  0}, { 0,  0}, { 0,  0}, { 0,  0}}});
    drawRRectType({160,  10, 210,  30}, {{{25, 10}, {25, 10}, {25, 10}, {25, 10}}});
    drawRRectType({ 20,  80,  70, 100}, {{{ 5,  5}, { 5,  5}, { 5,  5}, { 5,  5}}});
    drawRRectType({ 90,  80, 140, 100}, {{{ 5,  5}, {10,  5}, {10,  5}, { 5,  5}}});
    drawRRectType({160,  80, 210, 100}, {{{ 5,  5}, {10,  5}, { 5,  5}, { 5,  5}}});
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
