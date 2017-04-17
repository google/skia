/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkImage.h"
#include "SkRRect.h"

static void rotated_checkerboard_shader(SkPaint* paint,
                                        SkColor c1,
                                        SkColor c2,
                                        int size) {
    SkBitmap bm;
    bm.allocN32Pixels(2 * size, 2 * size);
    bm.eraseColor(c1);
    bm.eraseArea(SkIRect::MakeLTRB(0, 0, size, size), c2);
    bm.eraseArea(SkIRect::MakeLTRB(size, size, 2 * size, 2 * size), c2);
    SkMatrix matrix;
    matrix.setScale(0.75f, 0.75f);
    matrix.preRotate(30.0f);
    paint->setShader(
            SkShader::MakeBitmapShader(bm, SkShader::kRepeat_TileMode, SkShader::kRepeat_TileMode,
                                       &matrix));
}

static void exercise_draw_pos_text(SkCanvas* canvas,
                                   const char* text,
                                   SkScalar x, SkScalar y,
                                   const SkPaint& paint) {
    size_t textLen = strlen(text);
    int count = paint.countText(text, textLen);
    SkAutoTArray<SkScalar> widths(count);
    paint.getTextWidths(text, textLen, &widths[0]);
    SkAutoTArray<SkPoint> pos(count);
    for (int i = 0; i < count; ++i) {
        pos[i].set(x, y);
        x += widths[i];
    }
    canvas->drawPosText(text, textLen, &pos[0], paint);
}

static void exercise_draw_pos_text_h(SkCanvas* canvas,
                                     const char* text,
                                     SkScalar x, SkScalar y,
                                     const SkPaint& paint) {
    size_t textLen = strlen(text);
    int count = paint.countText(text, textLen);
    SkAutoTArray<SkScalar> widths(count);
    paint.getTextWidths(text, textLen, &widths[0]);
    SkAutoTArray<SkScalar> pos(count);
    for (int i = 0; i < count; ++i) {
        pos[i] = x;
        x += widths[i];
    }
    canvas->drawPosTextH(text, textLen, &pos[0], y, paint);
}

static void test_text(SkCanvas* canvas, SkScalar size,
                      SkColor color, SkScalar Y) {
    SkPaint type;
    type.setTextSize(24);
    sk_tool_utils::set_portable_typeface(&type);
    type.setColor(color);
    const char text[] = "HELLO WORLD";
    canvas->drawString(text, 32, size / 2 + Y, type);
    SkScalar lineSpacing = type.getFontSpacing();
    exercise_draw_pos_text(canvas, text, 32, size / 2 + Y + lineSpacing, type);
    exercise_draw_pos_text_h(canvas, text, 32,
                             size / 2 + Y + 2 * lineSpacing, type);
}

// If this GM works correctly, the cyan layer should be lined up with
// the objects below it.
DEF_SIMPLE_GM(skbug_257, canvas, 512, 512) {
    const SkScalar size = 256;
    SkAutoCanvasRestore autoCanvasRestore0(canvas, true);
    const SkScalar scale = 1.00168f;
    canvas->scale(scale, scale);
    {
        SkPaint checker;
        rotated_checkerboard_shader(&checker, SK_ColorWHITE, SK_ColorBLACK, 16);
        checker.setAntiAlias(true);

        SkAutoCanvasRestore autoCanvasRestore(canvas, true);
        canvas->clear(0xFFCECFCE);
        SkScalar translate = 225364.0f;
        canvas->translate(0, -translate);

        // Test rects
        SkRect rect = SkRect::MakeLTRB(8, 8 + translate, size - 8,
                                       size - 8 + translate);
        canvas->drawRect(rect, checker);

        // Test Paths
        canvas->translate(size, 0);
        SkRRect rrect;
        SkVector radii[4] = {{40, 40}, {40, 40}, {40, 40}, {40, 40}};
        rrect.setRectRadii(rect, radii);
        canvas->drawRRect(rrect, checker);

        // Test Points
        canvas->translate(-size, size);
        SkScalar delta = 1.0 / 64.0;
        SkPoint points[8] = {{size / 2, 8 + translate},
                             {size / 2, 8 + translate + delta},
                             {8, size / 2 + translate},
                             {8, size / 2 + translate + delta},
                             {size / 2, size - 8 + translate},
                             {size / 2, size - 8 + translate + delta},
                             {size - 8, size / 2 + translate},
                             {size - 8, size / 2 + translate + delta}};
        checker.setStyle(SkPaint::kStroke_Style);
        checker.setStrokeWidth(8);
        checker.setStrokeCap(SkPaint::kRound_Cap);
        canvas->drawPoints(SkCanvas::kLines_PointMode, 8, points, checker);

        // Test Text
        canvas->translate(size, 0);
        test_text(canvas, size, SK_ColorBLACK, translate);
    }
    // reference points (without the huge translations).
    SkPaint stroke;
    stroke.setStyle(SkPaint::kStroke_Style);
    stroke.setStrokeWidth(5);
    stroke.setColor(SK_ColorCYAN);
    canvas->drawCircle(size / 2, size / 2, size / 2 - 10, stroke);
    canvas->drawCircle(3 * size / 2, size / 2, size / 2 - 10, stroke);
    canvas->drawCircle(size / 2, 384, size / 2 - 10, stroke);
    canvas->translate(size, size);
    test_text(canvas, size, SK_ColorCYAN, 0.0f);
}
