/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "ToolUtils.h"
#include "gm.h"

#include "SkCanvas.h"
#include "SkFontPriv.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkRandom.h"
#include "SkTemplates.h"
#include "SkTextBlob.h"

static void strokePath(SkCanvas* canvas, const SkPath& path) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorRED);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawPath(path, paint);
}
DEF_SIMPLE_GM(getpostextpath, canvas, 480, 780) {
    // explicitly add spaces, to test a prev. bug
    const char* text = "Ham bur ge fons";
    size_t len = strlen(text);
    SkPath path;

    SkFont font;
    font.setTypeface(ToolUtils::create_portable_typeface());
    font.setSize(48);

    SkPaint paint;
    paint.setAntiAlias(true);

    canvas->translate(SkIntToScalar(10), SkIntToScalar(64));

    canvas->drawSimpleText(text, len, kUTF8_SkTextEncoding, 0, 0, font, paint);
    ToolUtils::get_text_path(font, text, len, kUTF8_SkTextEncoding, &path, nullptr);
    strokePath(canvas, path);
    path.reset();

    SkAutoToGlyphs atg(font, text, len, kUTF8_SkTextEncoding);
    const int count = atg.count();
    SkAutoTArray<SkPoint>  pos(count);
    SkAutoTArray<SkScalar> widths(count);
    font.getWidths(atg.glyphs(), count, &widths[0]);

    SkRandom rand;
    SkScalar x = SkIntToScalar(20);
    SkScalar y = SkIntToScalar(100);
    for (int i = 0; i < count; ++i) {
        pos[i].set(x, y + rand.nextSScalar1() * 24);
        x += widths[i];
    }

    canvas->translate(0, SkIntToScalar(64));

    canvas->drawTextBlob(SkTextBlob::MakeFromPosText(text, len, &pos[0], font), 0, 0, paint);
    ToolUtils::get_text_path(font, text, len, kUTF8_SkTextEncoding, &path, &pos[0]);
    strokePath(canvas, path);
}
