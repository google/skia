/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkShader.h"
#include "include/core/SkString.h"
#include "include/private/base/SkTArray.h"
#include "include/utils/SkCamera.h"
#include "src/effects/SkEmbossMaskFilter.h"
#include "tools/DecodeUtils.h"
#include "tools/Resources.h"
#include "tools/timer/TimeUtils.h"
#include "tools/viewer/Slide.h"

using namespace skia_private;

namespace {
class CameraSlide : public Slide {
    TArray<sk_sp<SkShader>> fShaders;
    int fShaderIndex = 0;
    bool fFrontFace = false;
    SkScalar fRX = 0;
    SkScalar fRY = 0;
    SkSize fSize;

public:
    CameraSlide() { fName = "Camera"; }

    void load(SkScalar w, SkScalar h) override {
        fSize = {w, h};
        for (const char* resource : {
            "images/mandrill_512_q075.jpg",
            "images/dog.jpg",
            "images/gamut.png",
        }) {
            SkBitmap bm;
            if (ToolUtils::GetResourceAsBitmap(resource, &bm)) {
                SkRect src = { 0, 0, SkIntToScalar(bm.width()), SkIntToScalar(bm.height()) };
                SkRect dst = { -150, -150, 150, 150 };
                fShaders.push_back(bm.makeShader(SkSamplingOptions(SkFilterMode::kLinear),
                                                 SkMatrix::RectToRect(src, dst)));
            }
        }
    }

    void draw(SkCanvas* canvas) override {
        canvas->clear(0xFFDDDDDD);
        if (fShaders.size() > 0) {
            canvas->translate(fSize.width()/2, fSize.height()/2);

            Sk3DView    view;
            view.rotateX(fRX);
            view.rotateY(fRY);
            view.applyToCanvas(canvas);

            bool frontFace = view.dotWithNormal(0, 0, SK_Scalar1) < 0;
            if (frontFace != fFrontFace) {
                fFrontFace = frontFace;
                fShaderIndex = (fShaderIndex + 1) % fShaders.size();
            }

            SkPaint paint;
            paint.setAntiAlias(true);
            paint.setShader(fShaders[fShaderIndex]);
            SkRect r = { -150, -150, 150, 150 };
            canvas->drawRoundRect(r, 30, 30, paint);
        }
    }

    void resize(SkScalar w, SkScalar h) override { fSize = {w, h}; }

    bool animate(double nanos) override {
        fRY = nanos ? TimeUtils::Scaled(1e-9 * nanos, 90, 360) : 0;
        return true;
    }
};
}  // namespace
DEF_SLIDE( return new CameraSlide(); )
