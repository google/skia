/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkPoint.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkDashPathEffect.h"
#include "tools/ToolUtils.h"

static void test_nulldev(SkCanvas* canvas) {
    SkBitmap bm;
    bm.setInfo(SkImageInfo::MakeN32Premul(30, 30));
    // notice: no pixels mom! be sure we don't crash
    // https://code.google.com/p/chromium/issues/detail?id=352616
    SkCanvas c(bm);

    SkBitmap src;
    src.allocN32Pixels(10, 10);
    src.eraseColor(SK_ColorRED);

    // ensure we don't crash
    c.writePixels(src, 0, 0);
}

static void draw_text_stroked(SkCanvas* canvas, const SkPaint& paint, const SkFont& font,
                              SkScalar strokeWidth) {
    SkPaint p(paint);
    SkPoint loc = { 20, 435 };

    if (strokeWidth > 0) {
        p.setStyle(SkPaint::kFill_Style);
        canvas->drawSimpleText("P", 1, kUTF8_SkTextEncoding, loc.fX, loc.fY - 225, font, p);
        canvas->drawTextBlob(SkTextBlob::MakeFromPosText("P", 1, &loc, font), 0, 0, p);
    }

    p.setColor(SK_ColorRED);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(strokeWidth);

    canvas->drawSimpleText("P", 1, kUTF8_SkTextEncoding, loc.fX, loc.fY - 225, font, p);
    canvas->drawTextBlob(SkTextBlob::MakeFromPosText("P", 1, &loc, font), 0, 0, p);
}

static void draw_text_set(SkCanvas* canvas, const SkPaint& paint, const SkFont& font) {
    SkAutoCanvasRestore acr(canvas, true);

    draw_text_stroked(canvas, paint, font, 10);

    canvas->translate(200, 0);
    draw_text_stroked(canvas, paint, font, 0);

    const SkScalar intervals[] = { 20, 10, 5, 10 };
    const SkScalar phase = 0;

    canvas->translate(200, 0);
    SkPaint p(paint);
    p.setPathEffect(SkDashPathEffect::Make(intervals, SK_ARRAY_COUNT(intervals), phase));
    draw_text_stroked(canvas, p, font, 10);
}

namespace {
    enum {
        kBelowThreshold_TextSize = 255,
        kAboveThreshold_TextSize = 257
    };
}

DEF_SIMPLE_GM(stroketext, canvas, 1200, 480) {
    if (true) { test_nulldev(canvas); }

    SkPaint paint;
    paint.setAntiAlias(true);

    SkFont font(ToolUtils::create_portable_typeface(), kBelowThreshold_TextSize);
    draw_text_set(canvas, paint, font);

    canvas->translate(600, 0);
    font.setSize(kAboveThreshold_TextSize);
    draw_text_set(canvas, paint, font);
}
