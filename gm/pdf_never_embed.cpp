/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkPaint.h"
#include "include/core/SkScalar.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "src/core/SkFontPriv.h"
#include "tools/Resources.h"
#include "tools/fonts/FontToolUtils.h"

#include <string.h>

static void excercise_draw_pos_text(SkCanvas* canvas,
                                    const char* text,
                                    SkScalar x, SkScalar y,
                                    const SkFont& font,
                                    const SkPaint& paint) {
    const size_t count = font.countText(text, strlen(text), SkTextEncoding::kUTF8);
    SkTextBlobBuilder builder;
    auto rec = builder.allocRunPos(font, count);
    font.textToGlyphs(text, strlen(text), SkTextEncoding::kUTF8, {rec.glyphs, count});
    font.getPos({rec.glyphs, count}, {rec.points(), count});
    canvas->drawTextBlob(builder.make(), x, y, paint);
}

DEF_SIMPLE_GM_CAN_FAIL(pdf_never_embed, canvas, errorMsg, 512, 512) {
    SkPaint p;

    sk_sp<SkTypeface> tf =
            ToolUtils::CreateTypefaceFromResource("fonts/Roboto2-Regular_NoEmbed.ttf");
    if (!tf) {
        tf = ToolUtils::DefaultPortableTypeface();
    }
    SkASSERT(tf);
    SkFont font(tf, 60);

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
    canvas->drawSimpleText(text, strlen(text), SkTextEncoding::kUTF8, 30, 700, font, p);
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

// See https://issues.skia.org/issues/40045290
// This has two uses. The first is to simply show the behavior of the various font managers.
// The second is to test the behavior of the PDF subsetter for OpenType fonts which are table based.
DEF_SIMPLE_GM_CAN_FAIL(pdf_table_based_subset, canvas, errorMsg, 512, 128) {
    SkPaint p;

    sk_sp<SkFontMgr> fm = ToolUtils::TestFontMgr();
    SkASSERT_RELEASE(fm);

    auto makeTypeface = [&fm](const char* resource, bool useStream) -> sk_sp<SkTypeface> {
        sk_sp<SkTypeface> tf = fm->makeFromStream(GetResourceAsStream(resource, useStream), 0);
        if (!tf) {
            tf = ToolUtils::DefaultPortableTypeface();
        }
        return tf;
    };
    sk_sp<SkTypeface> typefaces[] = {
        makeTypeface("fonts/SpiderSymbol.ttf", false),
        makeTypeface("fonts/SpiderSymbol.ttf", true),
        makeTypeface("fonts/SpiderSymbol.woff", false),
        makeTypeface("fonts/SpiderSymbol.woff", true),
        makeTypeface("fonts/SpiderSymbol.woff2", false),
        makeTypeface("fonts/SpiderSymbol.woff2", true),
    };

    SkPoint o = SkPoint::Make(10, 10);
    const unsigned char text[4] = {0xF0, 0x9F, 0x95, 0xB7};
    const uint32_t cluster = 0;
    const SkUnichar spiderCodePoint = 0xf021;
    for (auto&& tf : typefaces) {
        SkFont font(tf, 60);
        SkGlyphID g = font.unicharToGlyph(spiderCodePoint);
        if (g == 0) {
            // Avoid adding glyph 0 since it can be difficult to tell apart from the default.
            g = font.unicharToGlyph('S');
        }
        SkRect bounds = font.getBounds(g, &p);

        SkPoint pt = SkPoint::Make(-bounds.left(), -bounds.top());
        canvas->drawGlyphs({&g, 1}, {&pt, 1}, {&cluster, 1}, (const char(&)[4])text, o, font, p);
        o.offset(bounds.width() + 10, 0);
    }

    return skiagm::DrawResult::kOk;
}
