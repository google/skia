/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkMatrix44.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"
#include "include/utils/Sk3D.h"
#include "include/utils/SkRandom.h"
#include "samplecode/Sample.h"
#include "tools/Resources.h"

static SkMatrix44 inv(const SkMatrix44& m) {
    SkMatrix44 inverse;
    SkAssertResult(m.invert(&inverse));
    return inverse;
}

class Sample3DView : public Sample {
    float   fNear = 0.05f;
    float   fFar = 4;
    float   fAngle = SK_ScalarPI / 4;

    SkPoint3    fEye { 0, 0, 1.0f/tan(fAngle/2) - 1 };
    SkPoint3    fCOA { 0, 0, 0 };
    SkPoint3    fUp  { 0, 1, 0 };

    SkMatrix44  fRot;
    SkPoint3    fTrans;

    void rotate(float x, float y, float z) {
        SkMatrix44 r;
        if (x) {
            r.setRotateAboutUnit(1, 0, 0, x);
        } else if (y) {
            r.setRotateAboutUnit(0, 1, 0, y);
        } else {
            r.setRotateAboutUnit(0, 0, 1, z);
        }
        fRot.postConcat(r);
    }

public:
    SkMatrix44 get44(const SkRect& r) const {
        SkMatrix44  camera,
                    perspective,
                    translate,
                    viewport;

        SkScalar w = r.width();
        SkScalar h = r.height();

        Sk3Perspective(&perspective, fNear, fFar, fAngle);
        Sk3LookAt(&camera, fEye, fCOA, fUp);
        translate.setTranslate(fTrans.fX, fTrans.fY, fTrans.fZ);
        viewport.setScale(w*0.5f, h*0.5f, 1).postTranslate(r.centerX(), r.centerY(), 0);

        return viewport * perspective * camera * translate * fRot * inv(viewport);
    }

    bool onChar(SkUnichar uni) override {
        float delta = SK_ScalarPI / 30;
        switch (uni) {
            case '8': this->rotate( delta, 0, 0); return true;
            case '2': this->rotate(-delta, 0, 0); return true;
            case '4': this->rotate(0,  delta, 0); return true;
            case '6': this->rotate(0, -delta, 0); return true;
            case '-': this->rotate(0, 0,  delta); return true;
            case '+': this->rotate(0, 0, -delta); return true;

            case 'i': fTrans.fZ += 0.1f; SkDebugf("z %g\n", fTrans.fZ); return true;
            case 'k': fTrans.fZ -= 0.1f; SkDebugf("z %g\n", fTrans.fZ); return true;

            case 'n': fNear += 0.1f; SkDebugf("near %g\n", fNear); return true;
            case 'N': fNear -= 0.1f; SkDebugf("near %g\n", fNear); return true;
            case 'f': fFar  += 0.1f; SkDebugf("far  %g\n", fFar); return true;
            case 'F': fFar  -= 0.1f; SkDebugf("far  %g\n", fFar); return true;
            default: break;
        }
        return false;
    }
};

class SampleRR3D : public Sample3DView {
    SkRRect fRR;
    sk_sp<SkShader> fShader;

    SkString name() override { return SkString("rrect3d"); }

    void onOnceBeforeDraw() override {
        fRR = SkRRect::MakeRectXY({20, 20, 380, 380}, 50, 50);
        fShader = GetResourceAsImage("images/mandrill_128.png")
                        ->makeShader(SkMatrix::MakeScale(3, 3));
    }

    bool onChar(SkUnichar uni) override {
        switch (uni) {
            default: break;
        }
        return this->Sample3DView::onChar(uni);
    }

    void onDrawContent(SkCanvas* canvas) override {
        SkMatrix44 mx = this->get44({0, 0, 400, 400});

        SkPaint paint;
        paint.setColor({0.75, 0.75, 0.75, 1});
        canvas->drawRRect(fRR, paint);

        paint.setShader(fShader);

        canvas->save();
        canvas->concat(mx);
        canvas->drawRRect(fRR, paint);
        canvas->restore();
    }
};
DEF_SAMPLE( return new SampleRR3D(); )
