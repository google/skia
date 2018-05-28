/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "DecodeFile.h"
#include "SampleCode.h"
#include "SkAnimTimer.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkCamera.h"
#include "SkEmbossMaskFilter.h"
#include "SkGradientShader.h"
#include "SkPath.h"
#include "SkRandom.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkString.h"
#include "SkUtils.h"

class CameraView : public SampleView {
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
    // overrides from SkEventSink
    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Camera");
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

    bool onAnimate(const SkAnimTimer& timer) override {
        if (timer.isStopped()) {
            fRY = 0;
        } else {
            fRY = timer.scaled(90, 360);
        }
        return true;
    }

private:
    SkScalar fRX, fRY, fRZ;
    typedef SampleView INHERITED;
};
static SkView* MyFactory() { return new CameraView; }
static SkViewRegister reg(MyFactory);

//////////////////////////////////////////////////////////////////////////////

#include "Sk3D.h"

class Sk3DSample : public SampleView {
    float   fNear = 0.5;
    float   fFar = 4;
    float   fAngle = SK_ScalarPI / 4;

    SkPoint3    fEye { -0.3, -1, 4 };
    SkPoint3    fCOA { 0, 0, 0 };
    SkPoint3    fUp = { 0, 1, 0 };

    SkMatrix44  fMV;

    SkPoint3    fP3[8];

    float fRY = 0;

public:
    Sk3DSample() {
        int index = 0;
        for (float x = 0; x <= 1; ++x) {
            for (float y = 0; y <= 1; ++y) {
                for (float z = 0; z <= 1; ++z) {
                    fP3[index++] = { x, -y, -z };
                }
            }
        }
        fMV.setIdentity();//setTranslate(-0.5, -0.5, -0.5);
    }

protected:
    // overrides from SkEventSink
    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Sk3DSample");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void onDrawContent(SkCanvas* canvas) override {
        SkMatrix44  camera, perspective, mv = fMV;

        if (false) {
            SkMatrix44 rot;
            rot.setRotateDegreesAbout(1, 0, 0, fRY);
            mv.postConcat(rot);
        }
      //  fEye.fY = fRY;

        Sk3Perspective(&perspective, fNear, fFar, fAngle);
        Sk3LookAt(&camera, fEye, fCOA, fUp);
        mv.postConcat(camera);
        mv.postConcat(perspective);
        SkPoint pts[8];
        Sk3MapPts(pts, mv, fP3, 8);

        canvas->save();
        float w = canvas->getBaseLayerSize().width();
        float h = canvas->getBaseLayerSize().height();
        canvas->translate(w/2, h/2);
        float s = std::min(w, h);
        canvas->scale(s/4, s/4);

        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);

        SkPath cube;

        for (int i = 0; i < 8; ++i) {
        //    SkDebugf("[i] %g %g\n", pts[i].fX, pts[i].fY);
        }

        cube.moveTo(pts[0]);
        cube.lineTo(pts[2]);
        cube.lineTo(pts[6]);
        cube.lineTo(pts[4]);
        cube.close();

        cube.moveTo(pts[1]);
        cube.lineTo(pts[3]);
        cube.lineTo(pts[7]);
        cube.lineTo(pts[5]);
        cube.close();

        cube.moveTo(pts[0]);    cube.lineTo(pts[1]);
        cube.moveTo(pts[2]);    cube.lineTo(pts[3]);
        cube.moveTo(pts[4]);    cube.lineTo(pts[5]);
        cube.moveTo(pts[6]);    cube.lineTo(pts[7]);

        canvas->drawPath(cube, paint);

        {
            SkPoint3 src[4] = {
                { 0, 0, 0 }, { 3, 0, 0 }, { 0, -3, 0 }, { 0, 0, -3 },
            };
            SkPoint dst[4];
            mv.setConcat(perspective, camera);
            Sk3MapPts(dst, mv, src, 4);
            const char str[] = "XYZ";
            for (int i = 1; i <= 3; ++i) {
                canvas->drawLine(dst[0], dst[i], paint);
            }

            canvas->getTotalMatrix().mapPoints(dst, 4);

            canvas->restore();
            for (int i = 1; i <= 3; ++i) {
                canvas->drawText(&str[i-1], 1, dst[i].fX, dst[i].fY, paint);
            }
        }
    }

    bool onAnimate(const SkAnimTimer& timer) override {
        if (timer.isStopped()) {
            fRY = 0;
        } else {
            fRY = SkScalarSin(timer.scaled(1, SK_ScalarPI*2));
        }
        return true;
    }

    typedef SampleView INHERITED;
};
DEF_SAMPLE( return new Sk3DSample; )
