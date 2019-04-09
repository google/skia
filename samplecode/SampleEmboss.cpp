/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Sample.h"
#include "SkBlurMask.h"
#include "SkCanvas.h"
#include "SkEmbossMaskFilter.h"
#include "SkGradientShader.h"
#include "SkGraphics.h"
#include "SkPath.h"
#include "SkRandom.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUTF.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkTime.h"
#include "SkTypeface.h"

class EmbossView : public Sample {
    SkEmbossMaskFilter::Light   fLight;
public:
    EmbossView() {
        fLight.fDirection[0] = SK_Scalar1;
        fLight.fDirection[1] = SK_Scalar1;
        fLight.fDirection[2] = SK_Scalar1;
        fLight.fAmbient = 128;
        fLight.fSpecular = 16*2;
    }

protected:
    virtual bool onQuery(Sample::Event* evt) {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, "Emboss");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        SkPaint paint;

        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(SkIntToScalar(10));
        paint.setMaskFilter(SkEmbossMaskFilter::Make(SkBlurMask::ConvertRadiusToSigma(4), fLight));
        paint.setShader(SkShaders::Color(SK_ColorBLUE));
        paint.setDither(true);

        canvas->drawCircle(SkIntToScalar(50), SkIntToScalar(50),
                           SkIntToScalar(30), paint);
    }

private:

    typedef Sample INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new EmbossView(); )
