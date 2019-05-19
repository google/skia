/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkColorFilterImageFilter.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkTableColorFilter.h"

#include <math.h>
#include <utility>

static sk_sp<SkShader> make_shader0(int w, int h) {
    SkPoint pts[] = { {0, 0}, {SkIntToScalar(w), SkIntToScalar(h)} };
    SkColor colors[] = {
        SK_ColorBLACK, SK_ColorGREEN, SK_ColorCYAN,
        SK_ColorRED, 0, SK_ColorBLUE, SK_ColorWHITE
    };
    return SkGradientShader::MakeLinear(pts, colors, nullptr, SK_ARRAY_COUNT(colors),
                                        SkTileMode::kClamp);
}
static void make_bm0(SkBitmap* bm) {
    int W = 120;
    int H = 120;
    bm->allocN32Pixels(W, H);
    bm->eraseColor(SK_ColorTRANSPARENT);

    SkCanvas canvas(*bm);
    SkPaint paint;
    paint.setShader(make_shader0(W, H));
    canvas.drawPaint(paint);
}
static sk_sp<SkShader> make_shader1(int w, int h) {
    SkScalar cx = SkIntToScalar(w)/2;
    SkScalar cy = SkIntToScalar(h)/2;
    SkColor colors[] = {
        SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE,
    };
    return SkGradientShader::MakeRadial(SkPoint::Make(cx, cy), cx, colors, nullptr,
                                        SK_ARRAY_COUNT(colors), SkTileMode::kClamp);
}
static void make_bm1(SkBitmap* bm) {
    int W = 120;
    int H = 120;
    SkScalar cx = SkIntToScalar(W)/2;
    SkScalar cy = SkIntToScalar(H)/2;
    bm->allocN32Pixels(W, H);
    bm->eraseColor(SK_ColorTRANSPARENT);

    SkCanvas canvas(*bm);
    SkPaint paint;
    paint.setShader(make_shader1(W, H));
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

static sk_sp<SkColorFilter> make_null_cf() {
    return nullptr;
}

static sk_sp<SkColorFilter> make_cf0() {
    uint8_t table[256]; make_table0(table);
    return SkTableColorFilter::Make(table);
}
static sk_sp<SkColorFilter> make_cf1() {
    uint8_t table[256]; make_table1(table);
    return SkTableColorFilter::Make(table);
}
static sk_sp<SkColorFilter> make_cf2() {
    uint8_t table[256]; make_table2(table);
    return SkTableColorFilter::Make(table);
}
static sk_sp<SkColorFilter> make_cf3() {
    uint8_t table0[256]; make_table0(table0);
    uint8_t table1[256]; make_table1(table1);
    uint8_t table2[256]; make_table2(table2);
    return SkTableColorFilter::MakeARGB(nullptr, table0, table1, table2);
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


        static sk_sp<SkColorFilter> (*gColorFilterMakers[])() = {
            make_null_cf, make_cf0, make_cf1, make_cf2, make_cf3
        };
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
                paint.setColorFilter(gColorFilterMakers[i]());
                canvas->drawBitmap(bm, x, y, &paint);
            }

            paint.setColorFilter(nullptr);

            for (unsigned i = 0; i < SK_ARRAY_COUNT(gColorFilterMakers); ++i) {
                sk_sp<SkColorFilter> colorFilter1(gColorFilterMakers[i]());
                sk_sp<SkImageFilter> imageFilter1(SkColorFilterImageFilter::Make(
                            std::move(colorFilter1), nullptr));

                // Move down to the next line and draw it
                // each draw being at xOffset of the previous one
                y += yOffset;
                x = 0;
                for (unsigned j = 1; j < SK_ARRAY_COUNT(gColorFilterMakers); ++j) {
                    sk_sp<SkColorFilter> colorFilter2(gColorFilterMakers[j]());
                    sk_sp<SkImageFilter> imageFilter2(SkColorFilterImageFilter::Make(
                                std::move(colorFilter2), imageFilter1, nullptr));
                    paint.setImageFilter(std::move(imageFilter2));
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
DEF_GM( return new TableColorFilterGM; )

//////////////////////////////////////////////////////////////////////////////

class ComposeColorFilterGM : public skiagm::GM {
    enum {
        COLOR_COUNT = 3,
        MODE_COUNT = 4,
    };
    const SkColor*      fColors;
    const SkBlendMode*  fModes;
    SkString            fName;

public:
    ComposeColorFilterGM(const SkColor colors[], const SkBlendMode modes[],
                         const char suffix[])
        : fColors(colors), fModes(modes)
    {
        fName.printf("colorcomposefilter_%s", suffix);
    }

protected:
    virtual SkString onShortName() {
        return fName;
    }

    virtual SkISize onISize() {
        return SkISize::Make(790, 790);
    }

    virtual void onDraw(SkCanvas* canvas) {
        SkBitmap bm;
        make_bm1(&bm);

        canvas->drawColor(0xFFDDDDDD);

        const int MODES = MODE_COUNT * COLOR_COUNT;
        sk_sp<SkColorFilter> filters[MODES];
        int index = 0;
        for (int i = 0; i < MODE_COUNT; ++i) {
            for (int j = 0; j < COLOR_COUNT; ++j) {
                filters[index++] = SkColorFilters::Blend(fColors[j], fModes[i]);
            }
        }

        SkPaint paint;
        paint.setShader(make_shader1(50, 50));
        SkRect r = SkRect::MakeWH(50, 50);
        const SkScalar spacer = 10;

        canvas->translate(spacer, spacer);

        canvas->drawRect(r, paint); // orig

        for (int i = 0; i < MODES; ++i) {
            paint.setColorFilter(filters[i]);

            canvas->save();
            canvas->translate((i + 1) * (r.width() + spacer), 0);
            canvas->drawRect(r, paint);
            canvas->restore();

            canvas->save();
            canvas->translate(0, (i + 1) * (r.width() + spacer));
            canvas->drawRect(r, paint);
            canvas->restore();
        }

        canvas->translate(r.width() + spacer, r.width() + spacer);

        for (int y = 0; y < MODES; ++y) {
            canvas->save();
            for (int x = 0; x < MODES; ++x) {
                paint.setColorFilter(filters[y]->makeComposed(filters[x]));
                canvas->drawRect(r, paint);
                canvas->translate(r.width() + spacer, 0);
            }
            canvas->restore();
            canvas->translate(0, r.height() + spacer);
        }
    }

private:
    typedef GM INHERITED;
};

const SkColor gColors0[] = { SK_ColorCYAN, SK_ColorMAGENTA, SK_ColorYELLOW };
const SkBlendMode gModes0[] = {
    SkBlendMode::kOverlay,
    SkBlendMode::kDarken,
    SkBlendMode::kColorBurn,
    SkBlendMode::kExclusion,
};
DEF_GM( return new ComposeColorFilterGM(gColors0, gModes0, "wacky"); )

const SkColor gColors1[] = { 0x80FF0000, 0x8000FF00, 0x800000FF };
const SkBlendMode gModes1[] = {
    SkBlendMode::kSrcOver,
    SkBlendMode::kXor,
    SkBlendMode::kDstOut,
    SkBlendMode::kSrcATop,
};
DEF_GM( return new ComposeColorFilterGM(gColors1, gModes1, "alpha"); )
