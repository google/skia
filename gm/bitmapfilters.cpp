/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFilterQuality.h"
#include "include/core/SkFont.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "tools/ToolUtils.h"

static void make_bm(SkBitmap* bm) {
    const SkColor colors[4] = {
        SK_ColorRED, SK_ColorGREEN,
        SK_ColorBLUE, SK_ColorWHITE
    };
    SkPMColor colorsPM[4];
    for (size_t i = 0; i < SK_ARRAY_COUNT(colors); ++i) {
        colorsPM[i] = SkPreMultiplyColor(colors[i]);
    }
    bm->allocN32Pixels(2, 2, true);

    *bm->getAddr32(0, 0) = colorsPM[0];
    *bm->getAddr32(1, 0) = colorsPM[1];
    *bm->getAddr32(0, 1) = colorsPM[2];
    *bm->getAddr32(1, 1) = colorsPM[3];
}

static SkScalar draw_bm(SkCanvas* canvas, const SkBitmap& bm,
                        SkScalar x, SkScalar y, SkPaint* paint) {
    canvas->drawBitmap(bm, x, y, paint);
    return SkIntToScalar(bm.width()) * 5/4;
}

static SkScalar draw_set(SkCanvas* c, const SkBitmap& bm, SkScalar x,
                         SkPaint* p) {
    x += draw_bm(c, bm, x, 0, p);
    p->setFilterQuality(kLow_SkFilterQuality);
    x += draw_bm(c, bm, x, 0, p);
    p->setDither(true);
    return x + draw_bm(c, bm, x, 0, p);
}

static SkScalar draw_row(SkCanvas* canvas, const SkBitmap& bm) {
    SkAutoCanvasRestore acr(canvas, true);

    SkPaint paint;
    paint.setAntiAlias(true);

    SkScalar x = 0;
    const int scale = 32;

    SkFont      font(ToolUtils::create_portable_typeface());
    const char* name = ToolUtils::colortype_name(bm.colorType());
    canvas->drawString(name, x, SkIntToScalar(bm.height())*scale*5/8,
                       font, paint);
    canvas->translate(SkIntToScalar(48), 0);

    canvas->scale(SkIntToScalar(scale), SkIntToScalar(scale));

    x += draw_set(canvas, bm, 0, &paint);
    paint.reset();
    paint.setAlphaf(0.5f);
    draw_set(canvas, bm, x, &paint);
    return x * scale / 3;
}

class FilterGM : public skiagm::GM {
    void onOnceBeforeDraw() override {
        make_bm(&fBM32);
        ToolUtils::copy_to(&fBM4444, kARGB_4444_SkColorType, fBM32);
        ToolUtils::copy_to(&fBM16, kRGB_565_SkColorType, fBM32);
    }

public:
    SkBitmap    fBM4444, fBM16, fBM32;

    FilterGM() {
        this->setBGColor(0xFFDDDDDD);
    }

protected:
    SkString onShortName() override {
        return SkString("bitmapfilters");
    }

    SkISize onISize() override {
        return SkISize::Make(540, 250);
    }

    void onDraw(SkCanvas* canvas) override {
        SkScalar x = SkIntToScalar(10);
        SkScalar y = SkIntToScalar(10);

        canvas->translate(x, y);
        y = draw_row(canvas, fBM4444);
        canvas->translate(0, y);
        y = draw_row(canvas, fBM16);
        canvas->translate(0, y);
        draw_row(canvas, fBM32);
    }

private:
    typedef skiagm::GM INHERITED;
};
DEF_GM( return new FilterGM; )

//////////////////////////////////////////////////////////////////////////////

class TestExtractAlphaGM : public skiagm::GM {
    void onOnceBeforeDraw() override {
        // Make a bitmap with per-pixels alpha (stroked circle)
        fBitmap.allocN32Pixels(100, 100);
        SkCanvas canvas(fBitmap);
        canvas.clear(0);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(SK_ColorBLUE);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(20);

        canvas.drawCircle(50, 50, 39, paint);

        fBitmap.extractAlpha(&fAlpha);
    }

public:
    SkBitmap fBitmap, fAlpha;

protected:
    SkString onShortName() override {
        return SkString("extractalpha");
    }

    SkISize onISize() override {
        return SkISize::Make(540, 330);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setFilterQuality(kLow_SkFilterQuality);
        paint.setColor(SK_ColorRED);

        canvas->drawBitmap(fBitmap, 10, 10, &paint);    // should stay blue (ignore paint's color)
        canvas->drawBitmap(fAlpha, 120, 10, &paint);    // should draw red
    }

private:
    typedef skiagm::GM INHERITED;
};
DEF_GM( return new TestExtractAlphaGM; )
