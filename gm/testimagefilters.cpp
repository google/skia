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

static SkImageFilter* make0() {
    return SkDownSampleImageFilter::Create(SK_Scalar1 / 5);
}

static SkImageFilter* make1() {
    return SkOffsetImageFilter::Create(SkIntToScalar(16), SkIntToScalar(16));
}

static SkImageFilter* make2() {
    sk_sp<SkColorFilter> cf(SkColorFilter::MakeModeFilter(SK_ColorBLUE, SkXfermode::kSrcIn_Mode));
    return SkColorFilterImageFilter::Create(cf.get());
}

static SkImageFilter* make3() {
    return SkBlurImageFilter::Create(8, 0);
}

static SkImageFilter* make4() {
    sk_sp<SkImageFilter> outer(SkOffsetImageFilter::Create(SkIntToScalar(16), SkIntToScalar(16)));
    sk_sp<SkImageFilter> inner(SkDownSampleImageFilter::Create(SK_Scalar1 / 5));
    return SkComposeImageFilter::Make(std::move(outer), std::move(inner)).release();
}

static SkImageFilter* make5() {
    sk_sp<SkImageFilter> first(SkOffsetImageFilter::Create(SkIntToScalar(16), SkIntToScalar(16)));
    sk_sp<SkImageFilter> second(SkDownSampleImageFilter::Create(SK_Scalar1 / 5));
    return SkMergeImageFilter::Make(std::move(first), std::move(second)).release();
}

static SkImageFilter* make6() {
    sk_sp<SkImageFilter> outer(SkOffsetImageFilter::Create(SkIntToScalar(16), SkIntToScalar(16)));
    sk_sp<SkImageFilter> inner(SkDownSampleImageFilter::Create(SK_Scalar1 / 5));
    sk_sp<SkImageFilter> compose(SkComposeImageFilter::Make(std::move(outer), std::move(inner)));

    sk_sp<SkColorFilter> cf(SkColorFilter::MakeModeFilter(0x880000FF, SkXfermode::kSrcIn_Mode));
    sk_sp<SkImageFilter> blue(SkColorFilterImageFilter::Create(cf.get()));

    return SkMergeImageFilter::Make(std::move(compose), std::move(blue)).release();
}

static SkImageFilter* make7() {
    sk_sp<SkImageFilter> outer(SkOffsetImageFilter::Create(SkIntToScalar(16), SkIntToScalar(16)));
    sk_sp<SkImageFilter> inner(make3());
    sk_sp<SkImageFilter> compose(SkComposeImageFilter::Make(std::move(outer), std::move(inner)));

    sk_sp<SkColorFilter> cf(SkColorFilter::MakeModeFilter(0x880000FF, SkXfermode::kSrcIn_Mode));
    sk_sp<SkImageFilter> blue(SkColorFilterImageFilter::Create(cf.get()));

    return SkMergeImageFilter::Make(std::move(compose), std::move(blue)).release();
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

DEF_GM( return new TestImageFiltersGM; )
