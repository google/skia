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
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "src/core/SkBlurMask.h"

namespace skiagm {

// This GM exercises the blurred rect nine-patching special cases when the
// blurred rect is very large and/or very far from the origin.
// It creates a large blurred rect/rectori then renders the 4 corners and the
// middle.
class BigBlursGM : public GM {
public:
    BigBlursGM() {
        this->setBGColor(0xFFDDDDDD);
    }

protected:
    SkString getName() const override { return SkString("bigblurs"); }

    SkISize getISize() override { return SkISize::Make(kWidth, kHeight); }

    void onDraw(SkCanvas* canvas) override {
        constexpr int kBig = 65536;
        const SkScalar kSigma = SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(4));

        const SkRect bigRect = SkRect::MakeWH(SkIntToScalar(kBig), SkIntToScalar(kBig));
        SkRect insetRect = bigRect;
        insetRect.inset(20, 20);

        SkPath rectori = SkPathBuilder().addRect(bigRect)
                                        .addRect(insetRect, SkPathDirection::kCCW)
                                        .detach();

        // The blur extends 3*kSigma out from the big rect.
        // Offset the close-up windows so we get the entire blur
        const SkScalar kLeftTopPad  = 3*kSigma;   // use on left & up of big rect
        const SkScalar kRightBotPad = kCloseUpSize-3*kSigma; // use on right and bot sides

        // UL hand corners of the rendered closeups
        const SkPoint origins[] = {
            { -kLeftTopPad,          -kLeftTopPad           }, // UL
            {  kBig-kRightBotPad,    -kLeftTopPad           }, // UR
            {  kBig-kRightBotPad,     kBig-kRightBotPad     }, // LR
            { -kLeftTopPad,           kBig-kRightBotPad     }, // LL
            {  kBig/2-kCloseUpSize/2, kBig/2-kCloseUpSize/2 }, // center
        };

        SkPaint outlinePaint;
        outlinePaint.setColor(SK_ColorRED);
        outlinePaint.setStyle(SkPaint::kStroke_Style);

        SkPaint blurPaint;
        blurPaint.setAntiAlias(true);
        blurPaint.setColor(SK_ColorBLACK);

        int desiredX = 0, desiredY = 0;

        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j <= kLastEnum_SkBlurStyle; ++j) {
                blurPaint.setMaskFilter(SkMaskFilter::MakeBlur((SkBlurStyle)j, kSigma));

                for (int k = 0; k < (int)std::size(origins); ++k) {
                    canvas->save();

                    SkRect clipRect = SkRect::MakeXYWH(SkIntToScalar(desiredX),
                                                       SkIntToScalar(desiredY),
                                                       SkIntToScalar(kCloseUpSize),
                                                       SkIntToScalar(kCloseUpSize));

                    canvas->clipRect(clipRect);

                    canvas->translate(desiredX-origins[k].fX,
                                      desiredY-origins[k].fY);

                    if (0 == i) {
                        canvas->drawRect(bigRect, blurPaint);
                    } else {
                        canvas->drawPath(rectori, blurPaint);
                    }
                    canvas->restore();
                    canvas->drawRect(clipRect, outlinePaint);

                    desiredX += kCloseUpSize;
                }

                desiredX = 0;
                desiredY += kCloseUpSize;
            }
        }
    }

private:
    inline static constexpr int kCloseUpSize = 64;
    inline static constexpr int kWidth = 5 * kCloseUpSize;
    inline static constexpr int kHeight = 2 * (kLastEnum_SkBlurStyle + 1) * kCloseUpSize;

    using INHERITED = GM;
};

DEF_GM(return new BigBlursGM;)
}  // namespace skiagm
