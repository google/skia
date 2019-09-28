/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/effects/SkImageFilters.h"
#include "tools/ToolUtils.h"

#include <utility>

namespace skiagm {

class DisplacementMapGM : public GM {
public:
    DisplacementMapGM() {
        this->setBGColor(0xFF000000);
    }

protected:
    SkString onShortName() override {
        return SkString("displacement");
    }

    void onOnceBeforeDraw() override {
        fBitmap = ToolUtils::create_string_bitmap(80, 80, 0xFF884422, 15, 55, 96, "g");

        SkColor c1 = ToolUtils::color_to_565(0xFF244484);
        SkColor c2 = ToolUtils::color_to_565(0xFF804020);

        fCheckerboard =
                SkImage::MakeFromBitmap(ToolUtils::create_checkerboard_bitmap(80, 80, c1, c2, 8));
        fSmall  = SkImage::MakeFromBitmap(ToolUtils::create_checkerboard_bitmap(64, 64, c1, c2, 8));
        fLarge  = SkImage::MakeFromBitmap(ToolUtils::create_checkerboard_bitmap(96, 96, c1, c2, 8));
        fLargeW = SkImage::MakeFromBitmap(ToolUtils::create_checkerboard_bitmap(96, 64, c1, c2, 8));
        fLargeH = SkImage::MakeFromBitmap(ToolUtils::create_checkerboard_bitmap(64, 96, c1, c2, 8));
    }

    SkISize onISize() override {
        return SkISize::Make(600, 500);
    }

    void drawClippedBitmap(SkCanvas* canvas, int x, int y, const SkPaint& paint) const {
        canvas->save();
        canvas->translate(SkIntToScalar(x), SkIntToScalar(y));
        canvas->clipRect(SkRect::MakeWH(SkIntToScalar(fBitmap.width()), SkIntToScalar(fBitmap.height())));
        canvas->drawBitmap(fBitmap, 0, 0, &paint);
        canvas->restore();
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorBLACK);
        SkPaint paint;
        sk_sp<SkImageFilter> displ(SkImageFilters::Image(fCheckerboard));
        paint.setImageFilter(SkImageFilters::DisplacementMap(
                SkColorChannel::kR, SkColorChannel::kG, 0.0f, displ, nullptr));
        this->drawClippedBitmap(canvas, 0, 0, paint);
        paint.setImageFilter(SkImageFilters::DisplacementMap(
                SkColorChannel::kB, SkColorChannel::kA, 16.0f, displ, nullptr));
        this->drawClippedBitmap(canvas, 100, 0, paint);
        paint.setImageFilter(SkImageFilters::DisplacementMap(
                SkColorChannel::kR, SkColorChannel::kB, 32.0f, displ, nullptr));
        this->drawClippedBitmap(canvas, 200, 0, paint);
        paint.setImageFilter(SkImageFilters::DisplacementMap(
                SkColorChannel::kG, SkColorChannel::kA, 48.0f, displ, nullptr));
        this->drawClippedBitmap(canvas, 300, 0, paint);
        paint.setImageFilter(SkImageFilters::DisplacementMap(
                SkColorChannel::kR, SkColorChannel::kA, 64.0f, displ, nullptr));
        this->drawClippedBitmap(canvas, 400, 0, paint);

        paint.setImageFilter(SkImageFilters::DisplacementMap(
                SkColorChannel::kR, SkColorChannel::kG, 40.0f, displ, nullptr));
        this->drawClippedBitmap(canvas, 0, 100, paint);
        paint.setImageFilter(SkImageFilters::DisplacementMap(
                SkColorChannel::kB, SkColorChannel::kA, 40.0f, displ, nullptr));
        this->drawClippedBitmap(canvas, 100, 100, paint);
        paint.setImageFilter(SkImageFilters::DisplacementMap(
                SkColorChannel::kR, SkColorChannel::kB, 40.0f, displ, nullptr));
        this->drawClippedBitmap(canvas, 200, 100, paint);
        paint.setImageFilter(SkImageFilters::DisplacementMap(
                SkColorChannel::kG, SkColorChannel::kA, 40.0f, displ, nullptr));
        this->drawClippedBitmap(canvas, 300, 100, paint);
        paint.setImageFilter(SkImageFilters::DisplacementMap(
                SkColorChannel::kR, SkColorChannel::kA, 40.0f, displ, nullptr));
        this->drawClippedBitmap(canvas, 400, 100, paint);

        SkIRect cropRect = SkIRect::MakeXYWH(30, 30, 40, 40);
        paint.setImageFilter(SkImageFilters::DisplacementMap(
                SkColorChannel::kR, SkColorChannel::kG, 0.0f, displ, nullptr, &cropRect));
        this->drawClippedBitmap(canvas, 0, 200, paint);
        paint.setImageFilter(SkImageFilters::DisplacementMap(
            SkColorChannel::kB, SkColorChannel::kA, 16.0f, displ, nullptr, &cropRect));
        this->drawClippedBitmap(canvas, 100, 200, paint);
        paint.setImageFilter(SkImageFilters::DisplacementMap(
                SkColorChannel::kR, SkColorChannel::kB, 32.0f, displ, nullptr, &cropRect));
        this->drawClippedBitmap(canvas, 200, 200, paint);
        paint.setImageFilter(SkImageFilters::DisplacementMap(
                SkColorChannel::kG, SkColorChannel::kA, 48.0f, displ, nullptr, &cropRect));
        this->drawClippedBitmap(canvas, 300, 200, paint);
        paint.setImageFilter(SkImageFilters::DisplacementMap(
                SkColorChannel::kR, SkColorChannel::kA, 64.0f, displ, nullptr, &cropRect));
        this->drawClippedBitmap(canvas, 400, 200, paint);

        paint.setImageFilter(SkImageFilters::DisplacementMap(
                SkColorChannel::kR, SkColorChannel::kG, 40.0f, displ, nullptr, &cropRect));
        this->drawClippedBitmap(canvas, 0, 300, paint);
        paint.setImageFilter(SkImageFilters::DisplacementMap(
                SkColorChannel::kB, SkColorChannel::kA, 40.0f, displ, nullptr, &cropRect));
        this->drawClippedBitmap(canvas, 100, 300, paint);
        paint.setImageFilter(SkImageFilters::DisplacementMap(
                SkColorChannel::kR, SkColorChannel::kB, 40.0f, displ, nullptr, &cropRect));
        this->drawClippedBitmap(canvas, 200, 300, paint);
        paint.setImageFilter(SkImageFilters::DisplacementMap(
                SkColorChannel::kG, SkColorChannel::kA, 40.0f, displ, nullptr, &cropRect));
        this->drawClippedBitmap(canvas, 300, 300, paint);
        paint.setImageFilter(SkImageFilters::DisplacementMap(
                SkColorChannel::kR, SkColorChannel::kA, 40.0f, displ, nullptr, &cropRect));
        this->drawClippedBitmap(canvas, 400, 300, paint);

        // Test for negative scale.
        paint.setImageFilter(SkImageFilters::DisplacementMap(
                SkColorChannel::kG, SkColorChannel::kA, -40.0f, displ, nullptr));
        this->drawClippedBitmap(canvas, 500, 0, paint);

        // Tests for images of different sizes
        displ = SkImageFilters::Image(fSmall);
        paint.setImageFilter(SkImageFilters::DisplacementMap(
                SkColorChannel::kR, SkColorChannel::kG, 40.0f, std::move(displ), nullptr));
        this->drawClippedBitmap(canvas, 0, 400, paint);
        displ = SkImageFilters::Image(fLarge);
        paint.setImageFilter(SkImageFilters::DisplacementMap(
                SkColorChannel::kB, SkColorChannel::kA, 40.0f, std::move(displ), nullptr));
        this->drawClippedBitmap(canvas, 100, 400, paint);
        displ = SkImageFilters::Image(fLargeW);
        paint.setImageFilter(SkImageFilters::DisplacementMap(
                SkColorChannel::kR, SkColorChannel::kB, 40.0f, std::move(displ), nullptr));
        this->drawClippedBitmap(canvas, 200, 400, paint);
        displ = SkImageFilters::Image(fLargeH);
        paint.setImageFilter(SkImageFilters::DisplacementMap(
                SkColorChannel::kG, SkColorChannel::kA, 40.0f, std::move(displ), nullptr));
        this->drawClippedBitmap(canvas, 300, 400, paint);

        // Test for no given displacement input. In this case, both displacement
        // and color should use the same bitmap, given to SkCanvas::drawBitmap()
        // as an input argument.
        paint.setImageFilter(SkImageFilters::DisplacementMap(
                SkColorChannel::kG, SkColorChannel::kA, 40.0f, nullptr, nullptr));
        this->drawClippedBitmap(canvas, 400, 400, paint);
    }

private:
    SkBitmap fBitmap;
    sk_sp<SkImage> fCheckerboard, fSmall, fLarge, fLargeW, fLargeH;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new DisplacementMapGM;)
}
