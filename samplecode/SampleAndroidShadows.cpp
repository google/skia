
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint3.h"
#include "include/effects/SkBlurMaskFilter.h"
#include "include/pathops/SkPathOps.h"
#include "include/utils/SkCamera.h"
#include "include/utils/SkShadowUtils.h"
#include "samplecode/Sample.h"
#include "src/core/SkBlurMask.h"
#include "src/utils/SkUTF.h"
#include "tools/ToolUtils.h"
#include "tools/timer/TimeUtils.h"

////////////////////////////////////////////////////////////////////////////

class ShadowsView : public Sample {
    SkPath    fRectPath;
    SkPath    fRRPath;
    SkPath    fCirclePath;
    SkPath    fFunkyRRPath;
    SkPath    fCubicPath;
    SkPath    fStarPath;
    SkPath    fSquareRRectPath;
    SkPath    fWideRectPath;
    SkPath    fWideOvalPath;
    SkPath    fNotchPath;
    SkPath    fTabPath;

    SkPoint3  fLightPos;
    SkScalar  fZDelta;
    SkScalar  fAnimTranslate;
    SkScalar  fAnimAngle;
    SkScalar  fAnimAlpha;

    bool      fShowAmbient;
    bool      fShowSpot;
    bool      fUseAlt;
    bool      fShowObject;
    bool      fIgnoreShadowAlpha;
    bool      fDoAlphaAnimation;

public:
    ShadowsView()
        : fZDelta(0)
        , fAnimTranslate(0)
        , fAnimAngle(0)
        , fAnimAlpha(1)
        , fShowAmbient(true)
        , fShowSpot(true)
        , fUseAlt(false)
        , fShowObject(true)
        , fIgnoreShadowAlpha(false)
        , fDoAlphaAnimation(false) {}

protected:
    void onOnceBeforeDraw() override {
        fCirclePath.addCircle(0, 0, 50);
        fRectPath.addRect(SkRect::MakeXYWH(-100, -50, 200, 100));
        fRRPath.addRRect(SkRRect::MakeRectXY(SkRect::MakeXYWH(-100, -50, 200, 100), 4, 4));
        fFunkyRRPath.addRoundRect(SkRect::MakeXYWH(-50, -50, SK_Scalar1 * 100, SK_Scalar1 * 100),
                                  40 * SK_Scalar1, 20 * SK_Scalar1,
                                  SkPath::kCW_Direction);
        fCubicPath.cubicTo(100 * SK_Scalar1, 50 * SK_Scalar1,
                           20 * SK_Scalar1, 100 * SK_Scalar1,
                           0 * SK_Scalar1, 0 * SK_Scalar1);
        fStarPath.moveTo(0.0f, -50.0f);
        fStarPath.lineTo(14.43f, -25.0f);
        fStarPath.lineTo(43.30f, -25.0f);
        fStarPath.lineTo(28.86f, 0.0f);
        fStarPath.lineTo(43.30f, 25.0f);
        fStarPath.lineTo(14.43f, 25.0f);
        fStarPath.lineTo(0.0f, 50.0f);
        fStarPath.lineTo(-14.43f, 25.0f);
        fStarPath.lineTo(-43.30f, 25.0f);
        fStarPath.lineTo(-28.86f, 0.0f);
        fStarPath.lineTo(-43.30f, -25.0f);
        fStarPath.lineTo(-14.43f, -25.0f);
        fSquareRRectPath.addRRect(SkRRect::MakeRectXY(SkRect::MakeXYWH(-50, -50, 100, 100),
                                                      10, 10));
        fWideRectPath.addRect(SkRect::MakeXYWH(0, 0, 630, 70));
        fWideOvalPath.addOval(SkRect::MakeXYWH(0, 0, 630, 70));

        fNotchPath.moveTo(0, 80);
        fNotchPath.arcTo(SkRect::MakeLTRB(-20, 80, 20, 120), -90, -90, false);
        fNotchPath.lineTo(-75, 100);
        fNotchPath.lineTo(-75, -100);
        fNotchPath.lineTo(75, -100);
        fNotchPath.lineTo(75, 100);
        fNotchPath.arcTo(SkRect::MakeLTRB(-20, 80, 20, 120), 0, -90, false);

        fTabPath.moveTo(-75, -100);
        fTabPath.lineTo(75, -100);
        fTabPath.lineTo(75, 100);
        fTabPath.arcTo(SkRect::MakeLTRB(-20, 80, 20, 120), 0, 90, false);
        fTabPath.arcTo(SkRect::MakeLTRB(-20, 80, 20, 120), 90, 90, false);
        fTabPath.lineTo(-75, 100);

        fLightPos = SkPoint3::Make(350, 0, 600);
    }

    SkString name() override { return SkString("AndroidShadows"); }

    bool onChar(SkUnichar uni) override {
            bool handled = false;
            switch (uni) {
                case 'W':
                    fShowAmbient = !fShowAmbient;
                    handled = true;
                    break;
                case 'S':
                    fShowSpot = !fShowSpot;
                    handled = true;
                    break;
                case 'T':
                    fUseAlt = !fUseAlt;
                    handled = true;
                    break;
                case 'O':
                    fShowObject = !fShowObject;
                    handled = true;
                    break;
                case 'N':
                    fDoAlphaAnimation = !fDoAlphaAnimation;
                    if (!fDoAlphaAnimation) {
                        fAnimAlpha = 1;
                    }
                    handled = true;
                    break;
                case '>':
                    fZDelta += 0.5f;
                    handled = true;
                    break;
                case '<':
                    fZDelta -= 0.5f;
                    handled = true;
                    break;
                case '?':
                    fIgnoreShadowAlpha = !fIgnoreShadowAlpha;
                    handled = true;
                    break;
                default:
                    break;
            }
            if (handled) {
                return true;
            }
            return false;
    }

    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(0xFFDDDDDD);
    }

    void drawShadowedPath(SkCanvas* canvas, const SkPath& path,
                          const SkPoint3& zPlaneParams,
                          const SkPaint& paint, SkScalar ambientAlpha,
                          const SkPoint3& lightPos, SkScalar lightWidth, SkScalar spotAlpha) {
        if (fIgnoreShadowAlpha) {
            ambientAlpha = 1;
            spotAlpha = 1;
        }
        if (!fShowAmbient) {
            ambientAlpha = 0;
        }
        if (!fShowSpot) {
            spotAlpha = 0;
        }
        uint32_t flags = 0;
        if (fUseAlt) {
            flags |= SkShadowFlags::kGeometricOnly_ShadowFlag;
        }

        SkColor ambientColor = SkColorSetARGB(ambientAlpha * 255, 0, 0, 0);
        SkColor spotColor = SkColorSetARGB(spotAlpha * 255, 0, 0, 0);
        SkShadowUtils::DrawShadow(canvas, path, zPlaneParams, lightPos, lightWidth,
                                  ambientColor, spotColor, flags);

        if (fShowObject) {
            canvas->drawPath(path, paint);
        } else {
            SkPaint strokePaint;

            strokePaint.setColor(paint.getColor());
            strokePaint.setStyle(SkPaint::kStroke_Style);

            canvas->drawPath(path, strokePaint);
        }
    }

    void onDrawContent(SkCanvas* canvas) override {
        this->drawBG(canvas);
        const SkScalar kLightWidth = 800;
        const SkScalar kAmbientAlpha = 0.039f;
        const SkScalar kSpotAlpha = 0.19f;

        SkPaint paint;
        paint.setAntiAlias(true);

        SkPoint3 lightPos = fLightPos;
        SkPoint3 zPlaneParams = SkPoint3::Make(0, 0, 0);

        paint.setColor(SK_ColorWHITE);
        canvas->translate(200, 90);
        zPlaneParams.fZ = SkTMax(1.0f, 2 + fZDelta);
        this->drawShadowedPath(canvas, fRRPath, zPlaneParams, paint, fAnimAlpha*kAmbientAlpha,
                               lightPos, kLightWidth, fAnimAlpha*kSpotAlpha);

        paint.setColor(SK_ColorRED);
        canvas->translate(250, 0);
        zPlaneParams.fZ = SkTMax(1.0f, 8 + fZDelta);
        this->drawShadowedPath(canvas, fRectPath, zPlaneParams, paint, fAnimAlpha*kAmbientAlpha,
                               lightPos, kLightWidth, fAnimAlpha*kSpotAlpha);

        paint.setColor(SK_ColorBLUE);
        canvas->translate(-250, 110);
        zPlaneParams.fZ = SkTMax(1.0f, 12 + fZDelta);
        this->drawShadowedPath(canvas, fCirclePath, zPlaneParams, paint, fAnimAlpha*kAmbientAlpha,
                               lightPos, kLightWidth, fAnimAlpha*0.5f);

        paint.setColor(SK_ColorGREEN);
        canvas->translate(250, 0);
        zPlaneParams.fZ = SkTMax(1.0f, 64 + fZDelta);
        this->drawShadowedPath(canvas, fRRPath, zPlaneParams, paint, fAnimAlpha*kAmbientAlpha,
                               lightPos, kLightWidth, fAnimAlpha*kSpotAlpha);

        paint.setColor(SK_ColorYELLOW);
        canvas->translate(-250, 110);
        zPlaneParams.fZ = SkTMax(1.0f, 8 + fZDelta);
        this->drawShadowedPath(canvas, fFunkyRRPath, zPlaneParams, paint, fAnimAlpha*kAmbientAlpha,
                               lightPos, kLightWidth, fAnimAlpha*kSpotAlpha);

        paint.setColor(SK_ColorCYAN);
        canvas->translate(250, 0);
        zPlaneParams.fZ = SkTMax(1.0f, 16 + fZDelta);
        this->drawShadowedPath(canvas, fCubicPath, zPlaneParams, paint, fAnimAlpha*kAmbientAlpha,
                               lightPos, kLightWidth, fAnimAlpha*kSpotAlpha);

        paint.setColor(SK_ColorWHITE);
        canvas->translate(250, -180);
        zPlaneParams.fZ = SkTMax(1.0f, 8 + fZDelta);
        this->drawShadowedPath(canvas, fStarPath, zPlaneParams, paint,
                               kAmbientAlpha, lightPos, kLightWidth, kSpotAlpha);

        paint.setColor(SK_ColorWHITE);
        canvas->translate(150, 0);
        zPlaneParams.fZ = SkTMax(1.0f, 2 + fZDelta);
        this->drawShadowedPath(canvas, fNotchPath, zPlaneParams, paint,
                               kAmbientAlpha, lightPos, kLightWidth, kSpotAlpha);

        paint.setColor(SK_ColorWHITE);
        canvas->translate(200, 0);
        zPlaneParams.fZ = SkTMax(1.0f, 16 + fZDelta);
        this->drawShadowedPath(canvas, fTabPath, zPlaneParams, paint,
                               kAmbientAlpha, lightPos, kLightWidth, kSpotAlpha);

        // circular reveal
        SkPath tmpPath;
        SkPath tmpClipPath;
        tmpClipPath.addCircle(fAnimTranslate, 0, 60);
        Op(fSquareRRectPath, tmpClipPath, kIntersect_SkPathOp, &tmpPath);

        paint.setColor(SK_ColorMAGENTA);
        canvas->translate(-725, 240);
        zPlaneParams.fZ = SkTMax(1.0f, 32 + fZDelta);
        this->drawShadowedPath(canvas, tmpPath, zPlaneParams, paint, .1f,
                               lightPos, kLightWidth, .5f);

        // path ops bug
        SkPath tmpClipPathBug;
        tmpClipPathBug.addCircle(88.0344925f, 0, 60);
        Op(fSquareRRectPath, tmpClipPathBug, kIntersect_SkPathOp, &tmpPath);

        canvas->translate(250, 0);
        zPlaneParams.fZ = SkTMax(1.0f, 32 + fZDelta);
        this->drawShadowedPath(canvas, tmpPath, zPlaneParams, paint, .1f,
                               lightPos, kLightWidth, .5f);

        // perspective paths
        SkPoint pivot = SkPoint::Make(fWideRectPath.getBounds().width()/2,
                                      fWideRectPath.getBounds().height()/2);
        SkPoint translate = SkPoint::Make(100, 450);
        paint.setColor(SK_ColorWHITE);
        Sk3DView view;
        view.save();
        view.rotateX(fAnimAngle);
        SkMatrix persp;
        view.getMatrix(&persp);
        persp.preTranslate(-pivot.fX, -pivot.fY);
        persp.postTranslate(pivot.fX + translate.fX, pivot.fY + translate.fY);
        canvas->setMatrix(persp);
        SkScalar radians = SkDegreesToRadians(fAnimAngle);
        zPlaneParams = SkPoint3::Make(0,
                                      SkScalarSin(radians),
                                      SkTMax(1.0f, 16 + fZDelta) - SkScalarSin(radians)*pivot.fY);
        this->drawShadowedPath(canvas, fWideRectPath, zPlaneParams, paint, .1f,
                               lightPos, kLightWidth, .5f);

        pivot = SkPoint::Make(fWideOvalPath.getBounds().width() / 2,
                              fWideOvalPath.getBounds().height() / 2);
        translate = SkPoint::Make(100, 600);
        view.restore();
        view.save();
        view.rotateY(fAnimAngle);
        view.getMatrix(&persp);
        persp.preTranslate(-pivot.fX, -pivot.fY);
        persp.postTranslate(pivot.fX + translate.fX, pivot.fY + translate.fY);
        canvas->setMatrix(persp);
        zPlaneParams = SkPoint3::Make(-SkScalarSin(radians),
                                      0,
                                      SkTMax(1.0f, 32 + fZDelta) + SkScalarSin(radians)*pivot.fX);
        this->drawShadowedPath(canvas, fWideOvalPath, zPlaneParams, paint, .1f,
                               lightPos, kLightWidth, .5f);

        pivot = SkPoint::Make(fStarPath.getBounds().width() / 2,
                              fStarPath.getBounds().height() / 2);
        translate = SkPoint::Make(700, 250);
        view.restore();
        view.rotateY(fAnimAngle);
        view.getMatrix(&persp);
        persp.preTranslate(-pivot.fX, -pivot.fY);
        persp.postTranslate(pivot.fX + translate.fX, pivot.fY + translate.fY);
        canvas->setMatrix(persp);
        zPlaneParams = SkPoint3::Make(-SkScalarSin(radians),
                                      0,
                                      SkTMax(1.0f, 8 + fZDelta) + SkScalarSin(radians)*pivot.fX);
        this->drawShadowedPath(canvas, fStarPath, zPlaneParams, paint, .1f,
                               lightPos, kLightWidth, .5f);
    }

    bool onAnimate(double nanos) override {
        fAnimTranslate = TimeUtils::PingPong(1e-9 * nanos, 30, 0, 125, -125);
        fAnimAngle = TimeUtils::PingPong(1e-9 * nanos, 15, 0, 0, 20);
        if (fDoAlphaAnimation) {
            fAnimAlpha = TimeUtils::PingPong(1e-9 * nanos, 5, 0, 1, 0);
        }
        return true;
    }

private:
    typedef Sample INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new ShadowsView(); )
