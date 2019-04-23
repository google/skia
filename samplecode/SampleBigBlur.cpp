/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkCanvas.h"
#include "include/core/SkMaskFilter.h"
#include "samplecode/Sample.h"
#include "src/core/SkBlurMask.h"

class BigBlurView : public Sample {
public:
    BigBlurView() {
    }

protected:
    virtual bool onQuery(Sample::Event* evt) {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, "BigBlur");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        SkPaint paint;
        canvas->save();
        paint.setColor(SK_ColorBLUE);
        paint.setMaskFilter(SkMaskFilter::MakeBlur(
            kNormal_SkBlurStyle,
            SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(128))));
        canvas->translate(200, 200);
        canvas->drawCircle(100, 100, 200, paint);
        canvas->restore();
    }

private:
    typedef Sample INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new BigBlurView(); )
