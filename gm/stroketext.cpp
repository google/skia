/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"

static void test_nulldev(SkCanvas* canvas) {
    SkBitmap bm;
    bm.setConfig(SkBitmap::kARGB_8888_Config, 30, 30);
    // notice: no pixels mom! be sure we don't crash
    // https://code.google.com/p/chromium/issues/detail?id=352616
    SkCanvas c(bm);

    SkBitmap src;
    src.allocN32Pixels(10, 10);
    src.eraseColor(SK_ColorRED);

    // ensure we don't crash
    c.writePixels(src, 0, 0);
}

static void draw_text_stroked(SkCanvas* canvas, const SkPaint& paint) {
    SkPaint p(paint);
    SkPoint loc = { 20, 450 };

    canvas->drawText("P", 1, loc.fX, loc.fY - 225, p);
    canvas->drawPosText("P", 1, &loc, p);

    p.setColor(SK_ColorRED);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(10);

    canvas->drawText("P", 1, loc.fX, loc.fY - 225, p);
    canvas->drawPosText("P", 1, &loc, p);
}

class StrokeTextGM : public skiagm::GM {
    // Skia has a threshold above which it draws text via paths instead of using scalercontext
    // and caching the glyph. This GM wants to ensure that we draw stroking correctly on both
    // sides of this threshold.
    enum {
        kBelowThreshold_TextSize = 255,
        kAboveThreshold_TextSize = 257
    };
public:
    StrokeTextGM() {}

protected:
    virtual SkString onShortName() SK_OVERRIDE {
        return SkString("stroketext");
    }

    virtual SkISize onISize() SK_OVERRIDE {
        return SkISize::Make(640, 480);
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        if (true) { test_nulldev(canvas); }
        SkPaint paint;
        paint.setAntiAlias(true);

        paint.setTextSize(kBelowThreshold_TextSize);
        draw_text_stroked(canvas, paint);

        canvas->translate(200, 0);
        paint.setTextSize(kAboveThreshold_TextSize);
        draw_text_stroked(canvas, paint);
    }

private:
    typedef skiagm::GM INHERITED;
};

DEF_GM( return SkNEW(StrokeTextGM); )
