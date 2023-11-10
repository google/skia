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
#include "include/core/SkFontArguments.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkPoint.h"
#include "include/core/SkScalar.h"
#include "include/core/SkStream.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkDashPathEffect.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

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
        canvas->drawSimpleText("P", 1, SkTextEncoding::kUTF8, loc.fX, loc.fY - 225, font, p);
        canvas->drawTextBlob(SkTextBlob::MakeFromPosText("P", 1, &loc, font), 0, 0, p);
    }

    p.setColor(SK_ColorRED);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(strokeWidth);

    canvas->drawSimpleText("P", 1, SkTextEncoding::kUTF8, loc.fX, loc.fY - 225, font, p);
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
    p.setPathEffect(SkDashPathEffect::Make(intervals, std::size(intervals), phase));
    draw_text_stroked(canvas, p, font, 10);
}

namespace {
    enum {
        kBelowThreshold_TextSize = 255,
        kAboveThreshold_TextSize = 257
    };
}  // namespace

DEF_SIMPLE_GM(stroketext, canvas, 1200, 480) {
    if (true) { test_nulldev(canvas); }

    SkPaint paint;
    paint.setAntiAlias(true);

    SkFont font(ToolUtils::DefaultPortableTypeface(), kBelowThreshold_TextSize);
    draw_text_set(canvas, paint, font);

    canvas->translate(600, 0);
    font.setSize(kAboveThreshold_TextSize);
    draw_text_set(canvas, paint, font);
}

DEF_SIMPLE_GM_CAN_FAIL(stroketext_native, canvas, msg, 650, 420) {
    sk_sp<SkTypeface> ttf = MakeResourceAsTypeface("fonts/Stroking.ttf");
    sk_sp<SkTypeface> otf = MakeResourceAsTypeface("fonts/Stroking.otf");

    sk_sp<SkTypeface> overlap = []() -> sk_sp<SkTypeface>{
        std::unique_ptr<SkStreamAsset> variableStream(GetResourceAsStream("fonts/Variable.ttf"));
        if (!variableStream) {
            return nullptr;
        }
        const SkFontArguments::VariationPosition::Coordinate position[] = {
            { SkSetFourByteTag('w','g','h','t'), 721.0f },
        };
        SkFontArguments params;
        params.setVariationDesignPosition({position, std::size(position)});
        return SkFontMgr::RefDefault()->makeFromStream(std::move(variableStream), params);
    }();

    if (!ttf && !otf && !overlap) {
        msg->append("No support for ttf or otf.");
        return skiagm::DrawResult::kSkip;
    }

    SkPaint p;
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(10);
    p.setStrokeCap(SkPaint::kRound_Cap);
    p.setStrokeJoin(SkPaint::kRound_Join);
    p.setARGB(0xff, 0xbb, 0x00, 0x00);

    if (ttf) {
        /* Stroking.ttf is structured like:
            nothing U+25CB ○ (nothing inside)
            something U+25C9 ◉ (a tiny thing inside)
            - off (point off / empty quad with implicit end) (before U+207B ⁻ / after U+208B ₋)
            + on  (point on / empty line) (before U+207A ⁺ / after U+208A ₊)
            0 off off (two implicit quads) (before U+2070 ⁰ / after U+2080 ₀)
            1 off on  (quad with implicit close around) (before U+00B9 ¹ / after U+2081 ₁)
            2 on  off (quad with implicit close) (before U+00B2 ² / after U+2082 ₂)
            3 on  on  (empty line) (before U+00B3 ³ / after U+2083 ₃)
        */
        SkFont font(ttf, 100);
        canvas->drawString("○◉  ⁻₋⁺₊", 10, 100, font, p);
        canvas->drawString("⁰₀¹₁²₂³₃", 10, 200, font, p);
    }

    if (otf) {
        /* Stroking.otf is structured like:
            nothing U+25CB ○
            something U+25C9 ◉
            0 moveto, moveto (before U+2070 ⁰) (nothing there, FreeType ignores these)
            1 moveto, empty line, moveto (before U+00B9 ¹) (degenerate lineto)
            3 moveto, empty cubic, moveto (before U+00B3 ³) (degenerate cubicto)
            f moveto, empty flex, moveto (before U+1DA0 ᶠ) (degenerate flex)
        */
        SkFont font(otf, 100);
        canvas->drawString("○◉  ⁰¹³ᶠ", 10, 300, font, p);
    }

    if (overlap) {
        /* Variable.ttf is structured like:
            U+74 t (glyf outline has overlap flag)
            U+167 ŧ (glyf outline does not have overlap flag)
        */
        SkFont font(overlap, 100);
        p.setStrokeWidth(1);
        canvas->drawString("tŧ", 10, 400, font, p);
    }

    return skiagm::DrawResult::kOk;
}
