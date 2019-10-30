/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkShader.h"
#include "include/core/SkString.h"
#include "include/utils/SkCamera.h"
#include "samplecode/DecodeFile.h"
#include "samplecode/Sample.h"
#include "src/effects/SkEmbossMaskFilter.h"
#include "tools/Resources.h"
#include "tools/timer/TimeUtils.h"

namespace {
class CameraView : public Sample {
    SkTArray<sk_sp<SkShader>> fShaders;
    int fShaderIndex = 0;
    bool fFrontFace = false;
    SkScalar fRX = 0;
    SkScalar fRY = 0;

    SkString name() override { return SkString("Camera"); }

    void onOnceBeforeDraw() override {
        for (const char* resource : {
            "images/mandrill_512_q075.jpg",
            "images/dog.jpg",
            "images/gamut.png",
        }) {
            SkBitmap bm;
            if (GetResourceAsBitmap(resource, &bm)) {
                SkRect src = { 0, 0, SkIntToScalar(bm.width()), SkIntToScalar(bm.height()) };
                SkRect dst = { -150, -150, 150, 150 };
                SkMatrix matrix;
                matrix.setRectToRect(src, dst, SkMatrix::kFill_ScaleToFit);
                fShaders.push_back(bm.makeShader(&matrix));
            }
        }
        this->setBGColor(0xFFDDDDDD);
    }

    void onDrawContent(SkCanvas* canvas) override {
        if (fShaders.count() > 0) {
            canvas->translate(this->width()/2, this->height()/2);

            Sk3DView    view;
            view.rotateX(fRX);
            view.rotateY(fRY);
            view.applyToCanvas(canvas);

            bool frontFace = view.dotWithNormal(0, 0, SK_Scalar1) < 0;
            if (frontFace != fFrontFace) {
                fFrontFace = frontFace;
                fShaderIndex = (fShaderIndex + 1) % fShaders.count();
            }

            SkPaint paint;
            paint.setAntiAlias(true);
            paint.setShader(fShaders[fShaderIndex]);
            paint.setFilterQuality(kLow_SkFilterQuality);
            SkRect r = { -150, -150, 150, 150 };
            canvas->drawRoundRect(r, 30, 30, paint);
        }
    }

    bool onAnimate(double nanos) override {
        fRY = nanos ? TimeUtils::Scaled(1e-9 * nanos, 90, 360) : 0;
        return true;
    }
};
}  // namespace
DEF_SAMPLE( return new CameraView(); )
