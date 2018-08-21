/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Sample.h"
#include "SkBlurMask.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkGraphics.h"
#include "SkMaskFilter.h"
#include "SkPath.h"
#include "SkRandom.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUTF.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkTime.h"
#include "SkTypeface.h"

#include "SkOSFile.h"
#include "SkStream.h"

class TextAlphaView : public Sample {
public:
    TextAlphaView() {
        fByte = 0xFF;
    }

protected:
    bool onQuery(Sample::Event* evt) override {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, "TextAlpha");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void onDrawContent(SkCanvas* canvas) override {
        const char* str = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        SkPaint paint;
        SkScalar    x = SkIntToScalar(10);
        SkScalar    y = SkIntToScalar(20);

        paint.setFlags(0x105);

        paint.setARGB(fByte, 0xFF, 0xFF, 0xFF);

        paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle,
                                                   SkBlurMask::ConvertRadiusToSigma(3)));

        SkRandom rand;

        for (int ps = 6; ps <= 35; ps++) {
            paint.setColor(rand.nextU() | (0xFF << 24));
            paint.setTextSize(SkIntToScalar(ps));
            paint.setTextSize(SkIntToScalar(24));
            canvas->drawString(str, x, y, paint);
            y += paint.getFontMetrics(nullptr);
        }
    }

    Sample::Click* onFindClickHandler(SkScalar x, SkScalar y, unsigned) override {
        return new Click(this);
    }

    bool onClick(Click* click) override {
        int y = click->fICurr.fY;
        if (y < 0) {
            y = 0;
        } else if (y > 255) {
            y = 255;
        }
        fByte = y;
        return true;
    }

private:
    int fByte;

    typedef Sample INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new TextAlphaView(); )
