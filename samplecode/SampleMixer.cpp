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
#include "samplecode/Sample.h"
#include "src/core/SkUtils.h"
#include "tools/Resources.h"

const float gMat[] = {
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
    SkString name() override { return SkString("Mixer"); }

    void dodraw(SkCanvas* canvas, sk_sp<SkColorFilter> cf0, sk_sp<SkColorFilter> cf1, float gap) {
        SkPaint paint;
        paint.setColorFilter(cf0);
        canvas->drawImage(fImg, 0, 0, &paint);

        paint.setColorFilter(SkColorFilters::Lerp(fWeight, cf0, cf1));
        canvas->drawImage(fImg, fImg->width() + gap * fWeight, 0, &paint);

        paint.setColorFilter(cf1);
        canvas->drawImage(fImg, 2*fImg->width() + gap, 0, &paint);
    }

    void onDrawContent(SkCanvas* canvas) override {
        if (!fImg) {
            fImg = GetResourceAsImage("images/mandrill_256.png");
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

    virtual Click* onFindClickHandler(SkScalar x, SkScalar y, ModifierKey) override {
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
DEF_SAMPLE( return new MixerView; )

//////////////////////////////////////////////////////////////////////////////

#include "include/core/SkMaskFilter.h"
#include "include/core/SkSurface.h"

static sk_sp<SkShader> make_resource_shader(const char path[], int size) {
    auto img = GetResourceAsImage(path);
    if (!img) {
        return nullptr;
    }
    SkRect src = SkRect::MakeIWH(img->width(), img->height());
    SkRect dst = SkRect::MakeIWH(size, size);
    SkMatrix m;
    m.setRectToRect(src, dst, SkMatrix::kFill_ScaleToFit);
    return img->makeShader(&m);
}

class ShaderMixerView : public Sample {
    sk_sp<SkShader>     fSH0;
    sk_sp<SkShader>     fSH1;
    sk_sp<SkSurface>    fSurface;
    SkBlendMode         fMode = SkBlendMode::kClear;

    enum { SIZE = 256 };

    const SkRect fRect = SkRect::MakeXYWH(10, 10 + SIZE + 10, SIZE, SIZE);

public:
    ShaderMixerView() {}

    void onOnceBeforeDraw() override {
        fSH0 = make_resource_shader("images/mandrill_256.png", SIZE);
        fSH1 = make_resource_shader("images/baby_tux.png", SIZE);
    }

protected:
    SkString name() override { return SkString("ShaderMixer"); }

    void onDrawContent(SkCanvas* canvas) override {
        if (!fSurface) {
            fSurface = canvas->makeSurface(SkImageInfo::MakeN32Premul(SIZE, SIZE));
        }

        SkPaint paint;
        const SkRect r = SkRect::MakeIWH(SIZE, SIZE);

        canvas->translate(10, 10);

        canvas->save();
        paint.setShader(fSH0); canvas->drawRect(r, paint);
        canvas->translate(SIZE + 10.f, 0);
        paint.setShader(fSH1); canvas->drawRect(r, paint);
        canvas->restore();

        canvas->translate(0, SIZE + 10.f);

        auto sh = fSurface->makeImageSnapshot()->makeShader();

        canvas->save();
        paint.setShader(sh); canvas->drawRect(r, paint);
        canvas->translate(SIZE + 10.f, 0);
        paint.setShader(SkShaders::Lerp(sh, fSH0, fSH1)); canvas->drawRect(r, paint);
        canvas->restore();
    }

    virtual Click* onFindClickHandler(SkScalar x, SkScalar y, ModifierKey) override {
        fMode = (fMode == SkBlendMode::kSrcOver) ? SkBlendMode::kClear : SkBlendMode::kSrcOver;
        return fRect.contains(SkScalarRoundToInt(x),
                              SkScalarRoundToInt(y)) ? new Click(this) : nullptr;
    }

    bool onClick(Click* click) override {
        SkPaint p;
        p.setAntiAlias(true);
        p.setColor(SK_ColorRED);
        p.setBlendMode(fMode);
        p.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 12));
        SkScalar x = click->fCurr.fX - fRect.fLeft;
        SkScalar y = click->fCurr.fY - fRect.fTop;
        fSurface->getCanvas()->drawCircle(x, y, 10, p);
        return true;
    }

private:
    typedef Sample INHERITED;
};
DEF_SAMPLE( return new ShaderMixerView; )
