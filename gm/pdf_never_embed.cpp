/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "include/private/SkTo.h"
#include "tools/Resources.h"

static void excercise_draw_pos_text(SkCanvas* canvas,
                                    const char* text,
                                    SkScalar x, SkScalar y,
                                    const SkFont& font,
                                    const SkPaint& paint) {
    const int count = font.countText(text, strlen(text), kUTF8_SkTextEncoding);
    SkTextBlobBuilder builder;
    auto rec = builder.allocRunPos(font, count);
    font.textToGlyphs(text, strlen(text), kUTF8_SkTextEncoding, rec.glyphs, count);
    font.getPos(rec.glyphs, count, rec.points());
    canvas->drawTextBlob(builder.make(), x, y, paint);
}

DEF_SIMPLE_GM_CAN_FAIL(pdf_never_embed, canvas, errorMsg, 512, 512) {
    SkPaint p;

    SkFont font(MakeResourceAsTypeface("fonts/Roboto2-Regular_NoEmbed.ttf"), 60);
    if (!font.getTypefaceOrDefault()) {
        *errorMsg = "Could not load fonts/Roboto2-Regular_NoEmbed.ttf. "
                    "Did you forget to set the resourcePath?";
        return skiagm::DrawResult::kFail;
    }

    const char text[] = "HELLO, WORLD!";

    canvas->drawColor(SK_ColorWHITE);
    excercise_draw_pos_text(canvas, text, 30, 90, font, p);

    canvas->save();
    canvas->rotate(45.0f);
    p.setColor(0xF0800000);
    excercise_draw_pos_text(canvas, text, 30, 45, font, p);
    canvas->restore();

    canvas->save();
    canvas->scale(1, 4.0);
    p.setColor(0xF0008000);
    excercise_draw_pos_text(canvas, text, 15, 70, font, p);
    canvas->restore();

    canvas->scale(1.0, 0.5);
    p.setColor(0xF0000080);
    canvas->drawSimpleText(text, strlen(text), kUTF8_SkTextEncoding, 30, 700, font, p);
    return skiagm::DrawResult::kOk;
}


// should draw completely white.
DEF_SIMPLE_GM(pdf_crbug_772685, canvas, 612, 792) {
    canvas->clipRect({-1, -1, 613, 793}, false);
    canvas->translate(-571, 0);
    canvas->scale(0.75, 0.75);
    canvas->clipRect({-1, -1, 613, 793}, false);
    canvas->translate(0, -816);
    canvas->drawRect({0, 0, 1224, 1500}, SkPaint());
}
