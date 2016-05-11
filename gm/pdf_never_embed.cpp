/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "SkTypeface.h"
#include "gm.h"

static void excercise_draw_pos_text(SkCanvas* canvas,
                                    const char* text,
                                    SkScalar x, SkScalar y,
                                    const SkPaint& paint) {
    size_t textLen = strlen(text);
    SkAutoTArray<SkScalar> widths(SkToInt(textLen));
    paint.getTextWidths(text, textLen, &widths[0]);
    SkAutoTArray<SkPoint> pos(SkToInt(textLen));
    for (int i = 0; i < SkToInt(textLen); ++i) {
        pos[i].set(x, y);
        x += widths[i];
    }
    canvas->drawPosText(text, textLen, &pos[0], paint);
}

DEF_SIMPLE_GM(pdf_never_embed, canvas, 512, 512) {
    SkPaint p;
    p.setTextSize(60);
    p.setTypeface(MakeResourceAsTypeface("fonts/Roboto2-Regular_NoEmbed.ttf"));
    p.setAntiAlias(true);

    if (!p.getTypeface()) {
        return;
    }

    const char text[] = "HELLO, WORLD!";

    canvas->drawColor(SK_ColorWHITE);
    excercise_draw_pos_text(canvas, text, 30, 90, p);

    canvas->save();
    canvas->rotate(45.0f);
    p.setColor(0xF0800000);
    excercise_draw_pos_text(canvas, text, 30, 45, p);
    canvas->restore();

    canvas->save();
    canvas->scale(1, 4.0);
    p.setColor(0xF0008000);
    excercise_draw_pos_text(canvas, text, 15, 70, p);
    canvas->restore();

    canvas->scale(1.0, 0.5);
    p.setColor(0xF0000080);
    canvas->drawText(text, strlen(text), 30, 700, p);
}
