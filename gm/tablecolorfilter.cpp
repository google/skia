/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkColorFilterImageFilter.h"
#include "SkGradientShader.h"
#include "SkTableColorFilter.h"

static void make_bm0(SkBitmap* bm) {
    int W = 120;
    int H = 120;
    bm->allocN32Pixels(W, H);
    bm->eraseColor(SK_ColorTRANSPARENT);

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
    bm->allocN32Pixels(W, H);
    bm->eraseColor(SK_ColorTRANSPARENT);

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
        table[i] = static_cast<uint8_t>(sqrtf(fi) * 255);
    }
}

static SkColorFilter* make_null_cf() {
    return NULL;
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
        return SkISize::Make(700, 1650);
    }

    virtual void onDraw(SkCanvas* canvas) {
        canvas->drawColor(0xFFDDDDDD);
        canvas->translate(20, 20);


        static SkColorFilter* (*gColorFilterMakers[])() = { make_null_cf, make_cf0, make_cf1,
                                                 make_cf2, make_cf3 };
        static void (*gBitmapMakers[])(SkBitmap*) = { make_bm0, make_bm1 };

        // This test will be done once for each bitmap with the results stacked vertically.
        // For a single bitmap the resulting image will be the following:
        //  - A first line with the original bitmap, followed by the image drawn once
        //  with each of the N color filters
        //  - N lines of the bitmap drawn N times, this will cover all N*N combinations of
        //  pair of color filters in order to test the collpsing of consecutive table
        //  color filters.
        //
        //  Here is a graphical representation of the result for 2 bitmaps and 2 filters
        //  with the number corresponding to the number of filters the bitmap goes through:
        //
        //  --bitmap1
        //  011
        //  22
        //  22
        //  --bitmap2
        //  011
        //  22
        //  22

        SkScalar x = 0, y = 0;
        for (size_t bitmapMaker = 0; bitmapMaker < SK_ARRAY_COUNT(gBitmapMakers); ++bitmapMaker) {
            SkBitmap bm;
            gBitmapMakers[bitmapMaker](&bm);

            SkScalar xOffset = SkScalar(bm.width() * 9 / 8);
            SkScalar yOffset = SkScalar(bm.height() * 9 / 8);

            // Draw the first element of the first line
            x = 0;
            SkPaint paint;
            canvas->drawBitmap(bm, x, y, &paint);

            // Draws the rest of the first line for this bitmap
            // each draw being at xOffset of the previous one
            for (unsigned i = 1; i < SK_ARRAY_COUNT(gColorFilterMakers); ++i) {
                x += xOffset;
                paint.setColorFilter(gColorFilterMakers[i]())->unref();
                canvas->drawBitmap(bm, x, y, &paint);
            }

            paint.setColorFilter(NULL);

            for (unsigned i = 0; i < SK_ARRAY_COUNT(gColorFilterMakers); ++i) {
                SkAutoTUnref<SkColorFilter> colorFilter1(gColorFilterMakers[i]());
                SkAutoTUnref<SkImageFilter> imageFilter1(SkColorFilterImageFilter::Create(
                            colorFilter1, NULL, NULL, 0));

                // Move down to the next line and draw it
                // each draw being at xOffset of the previous one
                y += yOffset;
                x = 0;
                for (unsigned j = 1; j < SK_ARRAY_COUNT(gColorFilterMakers); ++j) {
                    SkAutoTUnref<SkColorFilter> colorFilter2(gColorFilterMakers[j]());
                    SkAutoTUnref<SkImageFilter> imageFilter2(SkColorFilterImageFilter::Create(
                                colorFilter2, imageFilter1, NULL, 0));
                    paint.setImageFilter(imageFilter2);
                    canvas->drawBitmap(bm, x, y, &paint);
                    x += xOffset;
                }
            }

            // Move down one line to the beginning of the block for next bitmap
            y += yOffset;
        }
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static skiagm::GM* MyFactory(void*) { return new TableColorFilterGM; }
static skiagm::GMRegistry reg(MyFactory);
