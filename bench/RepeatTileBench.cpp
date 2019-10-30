/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "bench/Benchmark.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkPaint.h"
#include "include/core/SkShader.h"
#include "include/core/SkString.h"
#include "tools/ToolUtils.h"

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
    r.setWH(SkIntToScalar(w), SkIntToScalar(h));
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(SkIntToScalar(4));
    p.setColor(SK_ColorBLUE);
    canvas.drawRect(r, p);
}

class RepeatTileBench : public Benchmark {
    const SkAlphaType   fAlphaType;
    SkPaint             fPaint;
    SkString            fName;
    SkBitmap            fBitmap;
public:
    RepeatTileBench(SkColorType ct, SkAlphaType at = kPremul_SkAlphaType) : fAlphaType(at) {
        const int w = 50;
        const int h = 50;

        fBitmap.setInfo(SkImageInfo::Make(w, h, ct, at));
        fName.printf("repeatTile_%s_%c",
                     ToolUtils::colortype_name(ct),
                     kOpaque_SkAlphaType == at ? 'X' : 'A');
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onDelayedSetup() override {
        fBitmap.allocPixels();
        fBitmap.eraseColor(kOpaque_SkAlphaType == fAlphaType ? SK_ColorWHITE : 0);

        draw_into_bitmap(fBitmap);

        fPaint.setShader(fBitmap.makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat));
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
