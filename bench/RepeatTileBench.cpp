
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkBenchmark.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColorPriv.h"
#include "SkPaint.h"
#include "SkShader.h"
#include "SkString.h"

static const char* gConfigName[] = {
    "ERROR", "a1", "a8", "index8", "565", "4444", "8888"
};

static void draw_into_bitmap(const SkBitmap& bm) {
    const int w = bm.width();
    const int h = bm.height();

    SkCanvas canvas(bm);
    SkPaint p;
    p.setAntiAlias(true);
    p.setColor(SK_ColorRED);
    canvas.drawCircle(SkIntToScalar(w)/2, SkIntToScalar(h)/2,
                      SkIntToScalar(SkMin32(w, h))*3/8, p);

    SkRect r;
    r.set(0, 0, SkIntToScalar(w), SkIntToScalar(h));
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(SkIntToScalar(4));
    p.setColor(SK_ColorBLUE);
    canvas.drawRect(r, p);
}

static int conv_6_to_byte(int x) {
    return x * 0xFF / 5;
}

static int conv_byte_to_6(int x) {
    return x * 5 / 255;
}

static uint8_t compute_666_index(SkPMColor c) {
    int r = SkGetPackedR32(c);
    int g = SkGetPackedG32(c);
    int b = SkGetPackedB32(c);

    return conv_byte_to_6(r) * 36 + conv_byte_to_6(g) * 6 + conv_byte_to_6(b);
}

static void convert_to_index666(const SkBitmap& src, SkBitmap* dst,
                                bool isOpaque) {
    SkPMColor storage[216];
    SkPMColor* colors = storage;
    // rrr ggg bbb
    for (int r = 0; r < 6; r++) {
        int rr = conv_6_to_byte(r);
        for (int g = 0; g < 6; g++) {
            int gg = conv_6_to_byte(g);
            for (int b = 0; b < 6; b++) {
                int bb = conv_6_to_byte(b);
                *colors++ = SkPreMultiplyARGB(0xFF, rr, gg, bb);
            }
        }
    }
    SkAlphaType aType = isOpaque ? kOpaque_SkAlphaType : kPremul_SkAlphaType;
    SkColorTable* ctable = new SkColorTable(storage, 216, aType);
    dst->setConfig(SkBitmap::kIndex8_Config, src.width(), src.height());
    dst->allocPixels(ctable);
    ctable->unref();

    SkAutoLockPixels alps(src);
    SkAutoLockPixels alpd(*dst);

    for (int y = 0; y < src.height(); y++) {
        const SkPMColor* srcP = src.getAddr32(0, y);
        uint8_t* dstP = dst->getAddr8(0, y);
        for (int x = src.width() - 1; x >= 0; --x) {
            *dstP++ = compute_666_index(*srcP++);
        }
    }
}

class RepeatTileBench : public SkBenchmark {
    SkPaint          fPaint;
    SkString         fName;
    SkBitmap         fBitmap;
    bool             fIsOpaque;
    SkBitmap::Config fConfig;
public:
    RepeatTileBench(SkBitmap::Config c, bool isOpaque = false)  {
        const int w = 50;
        const int h = 50;
        fConfig = c;
        fIsOpaque = isOpaque;

        if (SkBitmap::kIndex8_Config == fConfig) {
            fBitmap.setConfig(SkBitmap::kARGB_8888_Config, w, h);
        } else {
            fBitmap.setConfig(fConfig, w, h);
        }
        fName.printf("repeatTile_%s_%c",
                     gConfigName[fBitmap.config()], isOpaque ? 'X' : 'A');
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return fName.c_str();
    }

    virtual void onPreDraw() SK_OVERRIDE {
        fBitmap.allocPixels();
        fBitmap.eraseColor(fIsOpaque ? SK_ColorWHITE : 0);
        fBitmap.setAlphaType(fIsOpaque ?
                             kOpaque_SkAlphaType : kPremul_SkAlphaType);

        draw_into_bitmap(fBitmap);

        if (SkBitmap::kIndex8_Config == fConfig) {
            SkBitmap tmp;
            convert_to_index666(fBitmap, &tmp, fIsOpaque);
            fBitmap = tmp;
        }

        SkShader* s = SkShader::CreateBitmapShader(fBitmap,
                                                   SkShader::kRepeat_TileMode,
                                                   SkShader::kRepeat_TileMode);
        fPaint.setShader(s)->unref();
    }


    virtual void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        SkPaint paint(fPaint);
        this->setupPaint(&paint);

        for (int i = 0; i < loops; i++) {
            canvas->drawPaint(paint);
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

DEF_BENCH(return new RepeatTileBench(SkBitmap::kARGB_8888_Config, true))
DEF_BENCH(return new RepeatTileBench(SkBitmap::kARGB_8888_Config, false))
DEF_BENCH(return new RepeatTileBench(SkBitmap::kRGB_565_Config))
DEF_BENCH(return new RepeatTileBench(SkBitmap::kIndex8_Config))
