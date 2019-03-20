/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "AnimTimer.h"
#include "DecodeFile.h"
#include "Sample.h"
#include "SkCamera.h"
#include "SkCanvas.h"
#include "SkEmbossMaskFilter.h"
#include "SkGradientShader.h"
#include "SkPath.h"
#include "SkRandom.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkString.h"
#include "SkUTF.h"

class CameraView : public Sample {
    SkTArray<sk_sp<SkShader>> fShaders;
    int     fShaderIndex;
    bool    fFrontFace;
public:
    CameraView() {
        fRX = fRY = fRZ = 0;
        fShaderIndex = 0;
        fFrontFace = false;

        for (int i = 0;; i++) {
            SkString str;
            str.printf("/skimages/elephant%d.jpeg", i);
            SkBitmap bm;
            if (decode_file(str.c_str(), &bm)) {
                SkRect src = { 0, 0, SkIntToScalar(bm.width()), SkIntToScalar(bm.height()) };
                SkRect dst = { -150, -150, 150, 150 };
                SkMatrix matrix;
                matrix.setRectToRect(src, dst, SkMatrix::kFill_ScaleToFit);

                fShaders.push_back(SkShader::MakeBitmapShader(bm,
                                                           SkShader::kClamp_TileMode,
                                                           SkShader::kClamp_TileMode,
                                                           &matrix));
            } else {
                break;
            }
        }
        this->setBGColor(0xFFDDDDDD);
    }

protected:
    bool onQuery(Sample::Event* evt) override {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, "Camera");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->translate(this->width()/2, this->height()/2);

        Sk3DView    view;
        view.rotateX(fRX);
        view.rotateY(fRY);
        view.applyToCanvas(canvas);

        SkPaint paint;
        if (fShaders.count() > 0) {
            bool frontFace = view.dotWithNormal(0, 0, SK_Scalar1) < 0;
            if (frontFace != fFrontFace) {
                fFrontFace = frontFace;
                fShaderIndex = (fShaderIndex + 1) % fShaders.count();
            }

            paint.setAntiAlias(true);
            paint.setShader(fShaders[fShaderIndex]);
            paint.setFilterQuality(kLow_SkFilterQuality);
            SkRect r = { -150, -150, 150, 150 };
            canvas->drawRoundRect(r, 30, 30, paint);
        }
    }

    bool onAnimate(const AnimTimer& timer) override {
        if (timer.isStopped()) {
            fRY = 0;
        } else {
            fRY = timer.scaled(90, 360);
        }
        return true;
    }

private:
    SkScalar fRX, fRY, fRZ;
    typedef Sample INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new CameraView(); )
