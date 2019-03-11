/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Sample.h"
#include "SkCanvas.h"
#include "SkColorFilter.h"
#include "SkGradientShader.h"
#include "SkImage.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "Resources.h"

const SkScalar gMat[] = {
    .3f, .6f, .1f, 0, 0,
    .3f, .6f, .1f, 0, 0,
    .3f, .6f, .1f, 0, 0,
      0,   0,   0, 1, 0,
};

class MixerView : public Sample {
    sk_sp<SkImage>          fImg;
    sk_sp<SkColorFilter>    fCF0;
    sk_sp<SkColorFilter>    fCF1;

    float fWeight = 0;
    float fDW = 0.02f;

public:
    MixerView() {}

protected:
    bool onQuery(Event* evt) override {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, "Mixer");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void dodraw(SkCanvas* canvas, sk_sp<SkColorFilter> cf0, sk_sp<SkColorFilter> cf1, float gap) {
        SkPaint paint;
        paint.setColorFilter(cf0);
        canvas->drawImage(fImg, 0, 0, &paint);

        paint.setColorFilter(SkColorFilter::MakeLerp(cf0, cf1, fWeight));
        canvas->drawImage(fImg, fImg->width() + gap * fWeight, 0, &paint);

        paint.setColorFilter(cf1);
        canvas->drawImage(fImg, 2*fImg->width() + gap, 0, &paint);
    }

    void onDrawContent(SkCanvas* canvas) override {
        if (!fImg) {
            fImg = GetResourceAsImage("images/mandrill_256.png");
            fCF0 = SkColorFilter::MakeMatrixFilterRowMajor255(gMat);
            fCF1 = SkColorFilter::MakeModeFilter(0xFF44CC88, SkBlendMode::kScreen);
        }

        float gap = fImg->width() * 3;

        canvas->translate(10, 10);
        dodraw(canvas, nullptr, fCF1, gap);
        canvas->translate(0, fImg->height() + 10);
        dodraw(canvas, fCF0, nullptr, gap);
        canvas->translate(0, fImg->height() + 10);
        dodraw(canvas, fCF0, fCF1, gap);

        fWeight += fDW;
        if (fWeight > 1 || fWeight < 0) {
            fDW = -fDW;
        }
    }

    virtual Click* onFindClickHandler(SkScalar x, SkScalar y, unsigned) override {
        return fRect.contains(SkScalarRoundToInt(x),
                              SkScalarRoundToInt(y)) ? new Click(this) : nullptr;
    }

    bool onClick(Click* click) override {
        fRect.offset(click->fICurr.fX - click->fIPrev.fX,
                     click->fICurr.fY - click->fIPrev.fY);
        return true;
    }

private:
    SkIRect fRect;

    typedef Sample INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new MixerView; )
