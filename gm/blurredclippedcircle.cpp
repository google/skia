/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"

namespace skiagm {

// This GM reproduces the precision artifacts seen in crbug.com/560651.
// It draws a largish blurred circle with its center clipped out.
class BlurredClippedCircleGM : public GM {
public:
    BlurredClippedCircleGM() {
        this->setBGColor(0xFFCCCCCC);
    }

protected:

    SkString onShortName() override {
        return SkString("blurredclippedcircle");
    }

    SkISize onISize() override {
        return SkISize::Make(kWidth, kHeight);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint whitePaint;
        whitePaint.setColor(SK_ColorWHITE);
        whitePaint.setBlendMode(SkBlendMode::kSrc);
        whitePaint.setAntiAlias(true);

        // This scale exercises precision limits in the circle blur effect (crbug.com/560651)
        constexpr float kScale = 2.0f;
        canvas->scale(kScale, kScale);

        canvas->save();
            SkRect clipRect1 = SkRect::MakeLTRB(0, 0, kWidth, kHeight);
            canvas->clipRect(clipRect1);

            canvas->save();

                canvas->clipRect(clipRect1);
                canvas->drawRect(clipRect1, whitePaint);

                canvas->save();

                    SkRect clipRect2 = SkRect::MakeLTRB(8, 8, 288, 288);
                    SkRRect clipRRect = SkRRect::MakeOval(clipRect2);
                    canvas->clipRRect(clipRRect, SkClipOp::kDifference, true);

                    SkRect r = SkRect::MakeLTRB(4, 4, 292, 292);
                    SkRRect rr = SkRRect::MakeOval(r);

                    SkPaint paint;

                    paint.setMaskFilter(SkMaskFilter::MakeBlur(
                                            kNormal_SkBlurStyle,
                                            1.366025f));
                    paint.setColorFilter(SkColorFilters::Blend(SK_ColorRED, SkBlendMode::kSrcIn));
                    paint.setAntiAlias(true);

                    canvas->drawRRect(rr, paint);

                canvas->restore();
            canvas->restore();
        canvas->restore();
    }

private:
    static constexpr int kWidth = 1164;
    static constexpr int kHeight = 802;

    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new BlurredClippedCircleGM;)
}  // namespace skiagm
