/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkColorFilter.h"
#include "SkGradientShader.h"
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

class MixerView : public SampleView {
    sk_sp<SkImage>          fImg;
    sk_sp<SkColorFilter>    fCF0;
    sk_sp<SkColorFilter>    fCF1;

    float fWeight = 0;
    float fDW = 0.05f;

public:
    MixerView() {}

protected:
    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Mixer");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->scale(2, 2);

        if (!fImg) {
            fImg = GetResourceAsImage("mandrill_256.png");
            fCF0 = SkColorFilter::MakeMatrixFilterRowMajor255(gMat);
        }

        canvas->drawImage(fImg, 10, 10, nullptr);

        SkPaint paint;
        paint.setColorFilter(fCF0);
        canvas->drawImage(fImg, 10 + 2*(fImg->width() + 10), 10, &paint);


        auto mixer = SkColorFilter::MakeMixer(fCF0, fCF1, fWeight);
        paint.setColorFilter(mixer);
        canvas->drawImage(fImg, 10 + 1*(fImg->width() + 10), 10, &paint);

        fWeight += fDW;
        if (fWeight > 1 || fWeight < 0) {
            fDW = -fDW;
        }
        this->inval(nullptr);
    }

    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y,
                                              unsigned modi) override {
        return fRect.contains(SkScalarRoundToInt(x),
                              SkScalarRoundToInt(y)) ? new Click(this) : nullptr;
    }

    bool onClick(Click* click) override {
        fRect.offset(click->fICurr.fX - click->fIPrev.fX,
                     click->fICurr.fY - click->fIPrev.fY);
        this->inval(nullptr);
        return true;
    }

private:
    SkIRect fRect;

    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new MixerView; }
static SkViewRegister reg(MyFactory);
