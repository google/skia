/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleCode.h"
#include "SkBlurMask.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkEmbossMaskFilter.h"
#include "SkGradientShader.h"
#include "SkGraphics.h"
#include "SkPath.h"
#include "SkRandom.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkTime.h"
#include "SkTypeface.h"
#include "SkXfermode.h"

class EmbossView : public SampleView {
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
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Emboss");
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
        paint.setShader(SkShader::MakeColorShader(SK_ColorBLUE));
        paint.setDither(true);

        canvas->drawCircle(SkIntToScalar(50), SkIntToScalar(50),
                           SkIntToScalar(30), paint);
    }

private:

    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new EmbossView; }
static SkViewRegister reg(MyFactory);
