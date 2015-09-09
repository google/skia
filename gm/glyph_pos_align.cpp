/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"

/**
 * This test exercises drawPosTextH and drawPosText with every text align.
 */
static const int kWidth = 480;
static const int kHeight = 600;
static const SkScalar kTextHeight = 64.0f;
static const int kMaxStringLength = 12;

static void drawTestCase(SkCanvas*, const char*, SkScalar, const SkPaint&);

DEF_SIMPLE_GM_BG(glyph_pos_align, canvas, kWidth, kHeight, SK_ColorBLACK) {
        SkPaint paint;
        paint.setTextSize(kTextHeight);
        paint.setFakeBoldText(true);
        const SkColor colors[] = { SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE };
        const SkPoint pts[] = {{0, 0}, {kWidth, kHeight}};
        SkAutoTUnref<SkShader> grad(SkGradientShader::CreateLinear(pts, colors, nullptr,
                                                                   SK_ARRAY_COUNT(colors),
                                                                   SkShader::kMirror_TileMode));
        paint.setShader(grad);


        paint.setTextAlign(SkPaint::kRight_Align);
        drawTestCase(canvas, "Right Align", kTextHeight, paint);

        paint.setTextAlign(SkPaint::kCenter_Align);
        drawTestCase(canvas, "Center Align", 4 * kTextHeight, paint);

        paint.setTextAlign(SkPaint::kLeft_Align);
        drawTestCase(canvas, "Left Align", 7 * kTextHeight, paint);
}

void drawTestCase(SkCanvas* canvas, const char* text, SkScalar y, const SkPaint& paint) {
        SkScalar widths[kMaxStringLength];
        SkScalar posX[kMaxStringLength];
        SkPoint pos[kMaxStringLength];
        int length = SkToInt(strlen(text));
        SkASSERT(length <= kMaxStringLength);

        paint.getTextWidths(text, length, widths);

        float originX;
        switch (paint.getTextAlign()) {
            case SkPaint::kRight_Align: originX = 1; break;
            case SkPaint::kCenter_Align: originX = 0.5f; break;
            case SkPaint::kLeft_Align: originX = 0; break;
            default: SkFAIL("Invalid paint origin"); return;
        }

        float x = kTextHeight;
        for (int i = 0; i < length; ++i) {
            posX[i] = x + originX * widths[i];
            pos[i].set(posX[i], i ? pos[i - 1].y() + 3 : y + kTextHeight);
            x += widths[i];
        }

        canvas->drawPosTextH(text, length, posX, y, paint);
        canvas->drawPosText(text, length, pos, paint);
}
