/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkBlurMaskFilter.h"
#include "SkColorFilter.h"
#include "SkPaint.h"
#include "SkRRect.h"

namespace skiagm {

// This GM reproduces the precision artifacts seen in crbug.com/560651.
// It draws a largish blurred circle with its center clipped out.
class BlurredClippedCircleGM : public GM {
public:
    BlurredClippedCircleGM() {
        this->setBGColor(sk_tool_utils::color_to_565(0xFFCCCCCC));
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
        whitePaint.setXfermode(SkXfermode::Create(SkXfermode::kSrc_Mode))->unref();
        whitePaint.setAntiAlias(true);

        // This scale exercises precision limits in the circle blur effect (crbug.com/560651)
        static const float kScale = 2.0f;
        canvas->scale(kScale, kScale);

        canvas->save();
            SkRect clipRect1 = SkRect::MakeLTRB(0, 0,
                                                SkIntToScalar(kWidth), SkIntToScalar(kHeight));

            canvas->clipRect(clipRect1, SkRegion::kIntersect_Op, false);

            canvas->save();

                canvas->clipRect(clipRect1, SkRegion::kIntersect_Op, false);
                canvas->drawRect(clipRect1, whitePaint);

                canvas->save();

                    SkRect clipRect2 = SkRect::MakeLTRB(8, 8, 288, 288);
                    SkRRect clipRRect = SkRRect::MakeOval(clipRect2);
                    canvas->clipRRect(clipRRect, SkRegion::kDifference_Op, true);

                    SkRect r = SkRect::MakeLTRB(4, 4, 292, 292);
                    SkRRect rr = SkRRect::MakeOval(r);

                    SkPaint paint;

                    paint.setMaskFilter(SkBlurMaskFilter::Create(
                                            kNormal_SkBlurStyle,
                                            1.366025f,
                                            SkBlurMaskFilter::kHighQuality_BlurFlag))->unref();
                    paint.setColorFilter(SkColorFilter::CreateModeFilter(
                                             SK_ColorRED,
                                             SkXfermode::kSrcIn_Mode))->unref();
                    paint.setAntiAlias(true);

                    canvas->drawRRect(rr, paint);

                canvas->restore();
            canvas->restore();
        canvas->restore();
    }

private:
    static const int kWidth = 1164;
    static const int kHeight = 802;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new BlurredClippedCircleGM;)
}
