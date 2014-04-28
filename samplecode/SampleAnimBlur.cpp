
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SampleCode.h"
#include "SkBlurMaskFilter.h"
#include "SkColorPriv.h"
#include "SkCanvas.h"
#include "SkRandom.h"

class AnimBlurView : public SampleView {
public:
    AnimBlurView() {
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "AnimBlur");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    virtual void onDrawContent(SkCanvas* canvas) {

        SkScalar blurSigma = SampleCode::GetAnimSinScalar(100 * SK_Scalar1,
                                                          4 * SK_Scalar1,
                                                          5 * SK_Scalar1);

        SkScalar circleRadius = 3 * SK_Scalar1 +
                                SampleCode::GetAnimSinScalar(150 * SK_Scalar1,
                                                             25 * SK_Scalar1,
                                                             3 * SK_Scalar1);

        static const SkBlurStyle gStyles[] = {
            kNormal_SkBlurStyle,
            kInner_SkBlurStyle,
            kSolid_SkBlurStyle,
            kOuter_SkBlurStyle,
        };
        SkRandom random;

        for (size_t i = 0; i < SK_ARRAY_COUNT(gStyles); ++i) {
            SkMaskFilter* mf = SkBlurMaskFilter::Create(
                                       gStyles[i],
                                       blurSigma,
                                       SkBlurMaskFilter::kHighQuality_BlurFlag);
            SkPaint paint;
            SkSafeUnref(paint.setMaskFilter(mf));
            paint.setColor(random.nextU() | 0xff000000);
            canvas->drawCircle(200 * SK_Scalar1 + 400 * (i % 2) * SK_Scalar1,
                               200 * SK_Scalar1 + i / 2 * 400 * SK_Scalar1,
                               circleRadius, paint);
        }
        this->inval(NULL);
    }

private:
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new AnimBlurView; }
static SkViewRegister reg(MyFactory);
