/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkColorFilter.h"
#include "SkColorPriv.h"
#include "SkShader.h"

#include "SkBlurImageFilter.h"
#include "SkColorFilterImageFilter.h"
#include "SkComposeImageFilter.h"
#include "SkMergeImageFilter.h"
#include "SkOffsetImageFilter.h"
#include "SkTestImageFilters.h"

#define FILTER_WIDTH    SkIntToScalar(150)
#define FILTER_HEIGHT   SkIntToScalar(200)

static SkImageFilter* make0() { return SkDownSampleImageFilter::Create(SK_Scalar1 / 5); }
static SkImageFilter* make1() { return SkOffsetImageFilter::Create(SkIntToScalar(16), SkIntToScalar(16)); }
static SkImageFilter* make2() {
    SkColorFilter* cf = SkColorFilter::CreateModeFilter(SK_ColorBLUE,
                                                        SkXfermode::kSrcIn_Mode);
    SkAutoUnref aur(cf);
    return SkColorFilterImageFilter::Create(cf);
}
static SkImageFilter* make3() {
    return SkBlurImageFilter::Create(8, 0);
}

static SkImageFilter* make4() {
    SkImageFilter* outer = SkOffsetImageFilter::Create(SkIntToScalar(16), SkIntToScalar(16));
    SkImageFilter* inner = SkDownSampleImageFilter::Create(SK_Scalar1 / 5);
    SkAutoUnref aur0(outer);
    SkAutoUnref aur1(inner);
    return SkComposeImageFilter::Create(outer, inner);
}
static SkImageFilter* make5() {
    SkImageFilter* first = SkOffsetImageFilter::Create(SkIntToScalar(16), SkIntToScalar(16));
    SkImageFilter* second = SkDownSampleImageFilter::Create(SK_Scalar1 / 5);
    SkAutoUnref aur0(first);
    SkAutoUnref aur1(second);
    return SkMergeImageFilter::Create(first, second);
}

static SkImageFilter* make6() {
    SkImageFilter* outer = SkOffsetImageFilter::Create(SkIntToScalar(16), SkIntToScalar(16));
    SkImageFilter* inner = SkDownSampleImageFilter::Create(SK_Scalar1 / 5);
    SkAutoUnref aur0(outer);
    SkAutoUnref aur1(inner);
    SkImageFilter* compose = SkComposeImageFilter::Create(outer, inner);
    SkAutoUnref aur2(compose);

    SkColorFilter* cf = SkColorFilter::CreateModeFilter(0x880000FF,
                                                        SkXfermode::kSrcIn_Mode);
    SkAutoUnref aur3(cf);
    SkImageFilter* blue = SkColorFilterImageFilter::Create(cf);
    SkAutoUnref aur4(blue);

    return SkMergeImageFilter::Create(compose, blue);
}

static SkImageFilter* make7() {
    SkImageFilter* outer = SkOffsetImageFilter::Create(SkIntToScalar(16), SkIntToScalar(16));
    SkImageFilter* inner = make3();
    SkAutoUnref aur0(outer);
    SkAutoUnref aur1(inner);
    SkImageFilter* compose = SkComposeImageFilter::Create(outer, inner);
    SkAutoUnref aur2(compose);

    SkColorFilter* cf = SkColorFilter::CreateModeFilter(0x880000FF,
                                                        SkXfermode::kSrcIn_Mode);
    SkAutoUnref aur3(cf);
    SkImageFilter* blue = SkColorFilterImageFilter::Create(cf);
    SkAutoUnref aur4(blue);

    return SkMergeImageFilter::Create(compose, blue);
}

static void draw0(SkCanvas* canvas) {
    SkPaint p;
    p.setAntiAlias(true);
    SkRect r = SkRect::MakeWH(FILTER_WIDTH, FILTER_HEIGHT);
    r.inset(SK_Scalar1 * 12, SK_Scalar1 * 12);
    p.setColor(SK_ColorRED);
    canvas->drawOval(r, p);
}

class TestImageFiltersGM : public skiagm::GM {
public:
    TestImageFiltersGM () {}

protected:

    SkString onShortName() override {
        return SkString("testimagefilters");
    }

    SkISize onISize() override { return SkISize::Make(700, 460); }

    void onDraw(SkCanvas* canvas) override {
//        this->drawSizeBounds(canvas, 0xFFCCCCCC);

        static SkImageFilter* (*gFilterProc[])() = {
            make0, make1, make2, make3, make4, make5, make6, make7
        };

        const SkRect bounds = SkRect::MakeWH(FILTER_WIDTH, FILTER_HEIGHT);

        const SkScalar dx = bounds.width() * 8 / 7;
        const SkScalar dy = bounds.height() * 8 / 7;

        canvas->translate(SkIntToScalar(8), SkIntToScalar(8));

        for (int i = 0; i < (int)SK_ARRAY_COUNT(gFilterProc); ++i) {
            int ix = i % 4;
            int iy = i / 4;

            SkAutoCanvasRestore acr(canvas, true);
            canvas->translate(ix * dx, iy * dy);

            SkPaint p;
            p.setStyle(SkPaint::kStroke_Style);
            canvas->drawRect(bounds, p);

            SkPaint paint;
            paint.setImageFilter(gFilterProc[i]())->unref();
            canvas->saveLayer(&bounds, &paint);
            draw0(canvas);
        }
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static skiagm::GM* MyFactory(void*) { return new TestImageFiltersGM; }
static skiagm::GMRegistry reg(MyFactory);
