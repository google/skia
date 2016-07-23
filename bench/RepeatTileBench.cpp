/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Benchmark.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColorPriv.h"
#include "SkPaint.h"
#include "SkShader.h"
#include "SkString.h"
#include "sk_tool_utils.h"

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

static void convert_to_index666(const SkBitmap& src, SkBitmap* dst) {
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
    SkColorTable* ctable = new SkColorTable(storage, 216);
    dst->allocPixels(SkImageInfo::Make(src.width(), src.height(),
                                       kIndex_8_SkColorType, kOpaque_SkAlphaType),
                     nullptr, ctable);
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

class RepeatTileBench : public Benchmark {
    const SkColorType   fColorType;
    const SkAlphaType   fAlphaType;
    SkPaint             fPaint;
    SkString            fName;
    SkBitmap            fBitmap;
public:
    RepeatTileBench(SkColorType ct, SkAlphaType at = kPremul_SkAlphaType)
        : fColorType(ct), fAlphaType(at)
    {
        const int w = 50;
        const int h = 50;

        if (kIndex_8_SkColorType == ct) {
            fBitmap.setInfo(SkImageInfo::MakeN32(w, h, at));
        } else {
            fBitmap.setInfo(SkImageInfo::Make(w, h, ct, at));
        }
        fName.printf("repeatTile_%s_%c",
                     sk_tool_utils::colortype_name(ct), kOpaque_SkAlphaType == at ? 'X' : 'A');
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onDelayedSetup() override {
        fBitmap.allocPixels();
        fBitmap.eraseColor(kOpaque_SkAlphaType == fAlphaType ? SK_ColorWHITE : 0);

        draw_into_bitmap(fBitmap);

        if (kIndex_8_SkColorType == fColorType) {
            SkBitmap tmp;
            convert_to_index666(fBitmap, &tmp);
            fBitmap = tmp;
        }

        fPaint.setShader(SkShader::MakeBitmapShader(fBitmap,
                                                    SkShader::kRepeat_TileMode,
                                                    SkShader::kRepeat_TileMode));
    }


    void onDraw(int loops, SkCanvas* canvas) override {
        SkPaint paint(fPaint);
        this->setupPaint(&paint);

        for (int i = 0; i < loops; i++) {
            canvas->drawPaint(paint);
        }
    }

private:
    typedef Benchmark INHERITED;
};

DEF_BENCH(return new RepeatTileBench(kN32_SkColorType, kOpaque_SkAlphaType))
DEF_BENCH(return new RepeatTileBench(kN32_SkColorType, kPremul_SkAlphaType))
DEF_BENCH(return new RepeatTileBench(kRGB_565_SkColorType, kOpaque_SkAlphaType))
DEF_BENCH(return new RepeatTileBench(kIndex_8_SkColorType, kPremul_SkAlphaType))
