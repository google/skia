/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkBlurImageFilter.h"
#include "include/effects/SkColorFilterImageFilter.h"
#include "include/effects/SkDropShadowImageFilter.h"
#include "tools/ToolUtils.h"

///////////////////////////////////////////////////////////////////////////////

static void show_bounds(SkCanvas* canvas, const SkIRect& subset, const SkIRect& clip) {
    SkIRect rects[] { subset, clip };
    SkColor colors[] { SK_ColorRED, SK_ColorBLUE };

    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);

    for (size_t i = 0; i < SK_ARRAY_COUNT(rects); ++i) {
        paint.setColor(colors[i]);
        canvas->drawRect(SkRect::Make(rects[i]), paint);
    }
}

// In this GM, we're going to feed the inner portion of a 100x100 checkboard
// (i.e., strip off a 25-wide border) through the makeWithFilter method.
// We'll then draw the appropriate subset of the result to the screen at the
// given offset.
class ImageMakeWithFilterGM : public skiagm::GM {
public:
    ImageMakeWithFilterGM () {}

protected:
    SkString onShortName() override {
        return SkString("imagemakewithfilter");
    }

    SkISize onISize() override { return SkISize::Make(440, 530); }

    void onDraw(SkCanvas* canvas) override {
        auto cf = SkColorFilters::Blend(SK_ColorGREEN, SkBlendMode::kSrc);
        sk_sp<SkImageFilter> filters[] = {
            SkColorFilterImageFilter::Make(std::move(cf), nullptr),
            SkBlurImageFilter::Make(2.0f, 2.0f, nullptr),
            SkDropShadowImageFilter::Make(
                10.0f, 5.0f, 3.0f, 3.0f, SK_ColorBLUE,
                SkDropShadowImageFilter::kDrawShadowAndForeground_ShadowMode,
                nullptr),
        };

        SkIRect clipBounds[] {
            { -20, -20, 100, 100 },
            {   0,   0,  75,  75 },
            {  20,  20, 100, 100 },
            { -20, -20,  50,  50 },
            {  20,  20,  50,  50 },
        };

        SkImageInfo info = SkImageInfo::MakeN32(100, 100, kPremul_SkAlphaType);
        SkScalar MARGIN = SkIntToScalar(40);
        SkScalar DX = info.width() + MARGIN;
        SkScalar DY = info.height() + MARGIN;

        canvas->translate(MARGIN, MARGIN);

        sk_sp<SkSurface> surface = ToolUtils::makeSurface(canvas, info);
        ToolUtils::draw_checkerboard(surface->getCanvas());
        sk_sp<SkImage> source = surface->makeImageSnapshot();

        for (auto clipBound : clipBounds) {
            canvas->save();
            for (size_t i = 0; i < SK_ARRAY_COUNT(filters); ++i) {
                SkIRect subset = SkIRect::MakeXYWH(25, 25, 50, 50);
                SkIRect outSubset;
                SkIPoint offset;
                sk_sp<SkImage> result = source->makeWithFilter(filters[i].get(), subset, clipBound,
                                                               &outSubset, &offset);
                SkASSERT(result);
                SkASSERT(source->isTextureBacked() == result->isTextureBacked());
                result = result->makeSubset(outSubset);
                canvas->drawImage(result.get(), SkIntToScalar(offset.fX), SkIntToScalar(offset.fY));
                show_bounds(canvas, SkIRect::MakeXYWH(offset.x(), offset.y(), outSubset.width(),
                                                      outSubset.height()), clipBound);
                canvas->translate(DX, 0);
            }
            canvas->restore();
            canvas->translate(0, DY);
        }
    }

private:
    typedef GM INHERITED;
};
DEF_GM( return new ImageMakeWithFilterGM; )
