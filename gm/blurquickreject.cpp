/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFilterQuality.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "src/core/SkBlurMask.h"

// This GM tests out the quick reject bounds of the blur mask filter. It draws
// four blurred rects around a central clip. The blurred rect geometry outset
// by the blur radius does not overlap the clip rect so, if the blur clipping
// just uses the radius, they will be clipped out (and the result will differ
// from the result if quick reject were disabled. If the blur clipping uses
// the correct 3 sigma bound then the images with and without quick rejecting
// will be the same.
class BlurQuickRejectGM : public skiagm::GM {
public:
    BlurQuickRejectGM() {}

protected:
    SkString onShortName() override {
        return SkString("blurquickreject");
    }

    SkISize onISize() override {
        return SkISize::Make(kWidth, kHeight);
    }

    void onDraw(SkCanvas* canvas) override {
        constexpr SkScalar kBlurRadius = SkIntToScalar(20);
        constexpr SkScalar kBoxSize = SkIntToScalar(100);

        SkRect clipRect = SkRect::MakeXYWH(0, 0, kBoxSize, kBoxSize);
        SkRect blurRects[] = {
            { -kBoxSize - (kBlurRadius+1), 0, -(kBlurRadius+1), kBoxSize },
            { 0, -kBoxSize - (kBlurRadius+1), kBoxSize, -(kBlurRadius+1) },
            { kBoxSize+kBlurRadius+1, 0, 2*kBoxSize+kBlurRadius+1, kBoxSize },
            { 0, kBoxSize+kBlurRadius+1, kBoxSize, 2*kBoxSize+kBlurRadius+1 }
        };
        SkColor colors[] = {
            SK_ColorRED,
            SK_ColorGREEN,
            SK_ColorBLUE,
            SK_ColorYELLOW,
        };
        SkASSERT(SK_ARRAY_COUNT(colors) == SK_ARRAY_COUNT(blurRects));

        SkPaint hairlinePaint;
        hairlinePaint.setStyle(SkPaint::kStroke_Style);
        hairlinePaint.setColor(SK_ColorWHITE);
        hairlinePaint.setStrokeWidth(0);

        SkPaint blurPaint;
        blurPaint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle,
                                                    SkBlurMask::ConvertRadiusToSigma(kBlurRadius)));

        canvas->clear(SK_ColorBLACK);
        canvas->save();
        canvas->translate(kBoxSize, kBoxSize);
        canvas->drawRect(clipRect, hairlinePaint);
        canvas->clipRect(clipRect);
        for (size_t i = 0; i < SK_ARRAY_COUNT(blurRects); ++i) {
            blurPaint.setColor(colors[i]);
            canvas->drawRect(blurRects[i], blurPaint);
            canvas->drawRect(blurRects[i], hairlinePaint);
        }
        canvas->restore();
    }

private:
    static constexpr int kWidth = 300;
    static constexpr int kHeight = 300;

    using INHERITED = GM;
};

DEF_GM( return new BlurQuickRejectGM(); )
