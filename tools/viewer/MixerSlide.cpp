/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkImage.h"
#include "include/core/SkPath.h"
#include "include/core/SkRegion.h"
#include "include/core/SkShader.h"
#include "include/effects/SkGradientShader.h"
#include "src/base/SkUtils.h"
#include "tools/DecodeUtils.h"
#include "tools/Resources.h"
#include "tools/viewer/ClickHandlerSlide.h"

const float gMat[] = {
    .3f, .6f, .1f, 0, 0,
    .3f, .6f, .1f, 0, 0,
    .3f, .6f, .1f, 0, 0,
      0,   0,   0, 1, 0,
};

class MixerSlide : public ClickHandlerSlide {
    sk_sp<SkImage>          fImg;
    sk_sp<SkColorFilter>    fCF0;
    sk_sp<SkColorFilter>    fCF1;

    float fWeight = 0;
    float fDW = 0.02f;

public:
    MixerSlide() { fName = "Mixer"; }

    void draw(SkCanvas* canvas) override {
        if (!fImg) {
            fImg = ToolUtils::GetResourceAsImage("images/mandrill_256.png");
            fCF0 = SkColorFilters::Matrix(gMat);
            fCF1 = SkColorFilters::Blend(0xFF44CC88, SkBlendMode::kScreen);
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

protected:
    Click* onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey) override {
        return fRect.contains(SkScalarRoundToInt(x),
                              SkScalarRoundToInt(y)) ? new Click() : nullptr;
    }

    bool onClick(Click* click) override {
        fRect.offset(click->fCurr.fX - click->fPrev.fX,
                     click->fCurr.fY - click->fPrev.fY);
        return true;
    }

private:
    void dodraw(SkCanvas* canvas, sk_sp<SkColorFilter> cf0, sk_sp<SkColorFilter> cf1, float gap) {
        SkPaint paint;
        paint.setColorFilter(cf0);
        canvas->drawImage(fImg, 0, 0, SkSamplingOptions(), &paint);

        paint.setColorFilter(SkColorFilters::Lerp(fWeight, cf0, cf1));
        canvas->drawImage(fImg, fImg->width() + gap * fWeight, 0,
                          SkSamplingOptions(), &paint);

        paint.setColorFilter(cf1);
        canvas->drawImage(fImg, 2*fImg->width() + gap, 0, SkSamplingOptions(), &paint);
    }

    SkIRect fRect;

};
DEF_SLIDE( return new MixerSlide; )
