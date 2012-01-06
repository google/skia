/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkTableColorFilter.h"

static void make_bm0(SkBitmap* bm) {
    int W = 120;
    int H = 120;
    bm->setConfig(SkBitmap::kARGB_8888_Config, W, H);
    bm->allocPixels();
    bm->eraseColor(0);
    
    SkCanvas canvas(*bm);
    SkPaint paint;
    SkPoint pts[] = { {0, 0}, {SkIntToScalar(W), SkIntToScalar(H)} };
    SkColor colors[] = {
        SK_ColorBLACK, SK_ColorGREEN, SK_ColorCYAN,
        SK_ColorRED, 0, SK_ColorBLUE, SK_ColorWHITE
    };
    SkShader* s = SkGradientShader::CreateLinear(pts, colors, NULL, SK_ARRAY_COUNT(colors),
                                                 SkShader::kClamp_TileMode);
    paint.setShader(s)->unref();
    canvas.drawPaint(paint);
}
static void make_bm1(SkBitmap* bm) {
    int W = 120;
    int H = 120;
    bm->setConfig(SkBitmap::kARGB_8888_Config, W, H);
    bm->allocPixels();
    bm->eraseColor(0);
    
    SkCanvas canvas(*bm);
    SkPaint paint;
    SkScalar cx = SkIntToScalar(W)/2;
    SkScalar cy = SkIntToScalar(H)/2;
    SkColor colors[] = {
        SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE,
    };
    SkShader* s = SkGradientShader::CreateRadial(SkPoint::Make(SkIntToScalar(W)/2,
                                                               SkIntToScalar(H)/2),
                                                 SkIntToScalar(W)/2, colors, NULL, SK_ARRAY_COUNT(colors),
                                                 SkShader::kClamp_TileMode);
    paint.setShader(s)->unref();
    paint.setAntiAlias(true);
    canvas.drawCircle(cx, cy, cx, paint);
}

static void make_table0(uint8_t table[]) {
    for (int i = 0; i < 256; ++i) {
        int n = i >> 5;
        table[i] = (n << 5) | (n << 2) | (n >> 1);
    }
}
static void make_table1(uint8_t table[]) {
    for (int i = 0; i < 256; ++i) {
        table[i] = i * i / 255;
    }
}
static void make_table2(uint8_t table[]) {
    for (int i = 0; i < 256; ++i) {
        float fi = i / 255.0f;
        table[i] = sqrtf(fi) * 255;
    }
}

static SkColorFilter* make_cf0() {
    uint8_t table[256]; make_table0(table);
    return SkTableColorFilter::Create(table);
}
static SkColorFilter* make_cf1() {
    uint8_t table[256]; make_table1(table);
    return SkTableColorFilter::Create(table);
}
static SkColorFilter* make_cf2() {
    uint8_t table[256]; make_table2(table);
    return SkTableColorFilter::Create(table);
}
static SkColorFilter* make_cf3() {
    uint8_t table0[256]; make_table0(table0);
    uint8_t table1[256]; make_table1(table1);
    uint8_t table2[256]; make_table2(table2);
    return SkTableColorFilter::CreateARGB(NULL, table0, table1, table2);
}

class TableColorFilterGM : public skiagm::GM {
public:
    TableColorFilterGM() {}
    
protected:
    virtual SkString onShortName() {
        return SkString("tablecolorfilter");
    }
    
    virtual SkISize onISize() {
        return SkISize::Make(640, 480);
    }
    
    virtual void onDraw(SkCanvas* canvas) {
        canvas->drawColor(0xFFDDDDDD);
        canvas->translate(20, 20);
        
        SkScalar x = 0, y = 0;
        
        static void (*gMakers[])(SkBitmap*) = { make_bm0, make_bm1 };
        for (size_t maker = 0; maker < SK_ARRAY_COUNT(gMakers); ++maker) {
            SkBitmap bm;
            gMakers[maker](&bm);
            
            SkPaint paint;
            x = 0;
            canvas->drawBitmap(bm, x, y, &paint);
            paint.setColorFilter(make_cf0())->unref();  x += bm.width() * 9 / 8;
            canvas->drawBitmap(bm, x, y, &paint);
            paint.setColorFilter(make_cf1())->unref();  x += bm.width() * 9 / 8;
            canvas->drawBitmap(bm, x, y, &paint);
            paint.setColorFilter(make_cf2())->unref();  x += bm.width() * 9 / 8;
            canvas->drawBitmap(bm, x, y, &paint);
            paint.setColorFilter(make_cf3())->unref();  x += bm.width() * 9 / 8;
            canvas->drawBitmap(bm, x, y, &paint);
            
            y += bm.height() * 9 / 8;
        }
    }
    
private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static skiagm::GM* MyFactory(void*) { return new TableColorFilterGM; }
static skiagm::GMRegistry reg(MyFactory);
