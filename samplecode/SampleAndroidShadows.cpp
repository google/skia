
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SampleCode.h"
#include "SkAnimTimer.h"
#include "SkBlurMask.h"
#include "SkBlurMaskFilter.h"
#include "SkColorFilter.h"
#include "SkCamera.h"
#include "SkCanvas.h"
#include "SkPath.h"
#include "SkPathOps.h"
#include "SkPoint3.h"
#include "SkShadowUtils.h"
#include "SkUtils.h"
#include "SkView.h"
#include "sk_tool_utils.h"

////////////////////////////////////////////////////////////////////////////

class ShadowsView : public SampleView {
    SkPath    fRectPath;
    SkPath    fRRPath;
    SkPath    fCirclePath;
    SkPath    fFunkyRRPath;
    SkPath    fCubicPath;
    SkPath    fSquareRRectPath;
    SkPath    fWideRectPath;
    SkPath    fWideOvalPath;
    SkPoint3  fLightPos;
    SkScalar  fZDelta;
    SkScalar  fAnimTranslate;
    SkScalar  fAnimAngle;

    bool      fShowAmbient;
    bool      fShowSpot;
    bool      fUseAlt;
    bool      fShowObject;
    bool      fIgnoreShadowAlpha;

public:
    ShadowsView()
        : fZDelta(0)
        , fAnimTranslate(0)
        , fAnimAngle(0)
        , fShowAmbient(true)
        , fShowSpot(true)
        , fUseAlt(false)
        , fShowObject(true)
        , fIgnoreShadowAlpha(false) {}

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
        fSquareRRectPath.addRRect(SkRRect::MakeRectXY(SkRect::MakeXYWH(-50, -50, 100, 100),
                                                      10, 10));
        fWideRectPath.addRect(SkRect::MakeXYWH(0, 0, 630, 70));
        fWideOvalPath.addOval(SkRect::MakeXYWH(0, 0, 630, 70));

        fLightPos = SkPoint3::Make(350, 0, 600);
    }

    // overrides from SkEventSink
    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "AndroidShadows");
            return true;
        }

        SkUnichar uni;
        if (SampleCode::CharQ(*evt, &uni)) {
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
                this->inval(nullptr);
                return true;
            }
        }
        return this->INHERITED::onQuery(evt);
    }

    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(0xFFDDDDDD);
    }

    void drawShadowedPath(SkCanvas* canvas, const SkPath& path,
                          const SkPoint3& zPlaneParams,
                          const SkPaint& paint, SkScalar ambientAlpha,
                          const SkPoint3& lightPos, SkScalar lightWidth, SkScalar spotAlpha) {
        if (fIgnoreShadowAlpha) {
            ambientAlpha = 255;
            spotAlpha = 255;
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

        SkShadowUtils::DrawShadow(canvas, path, zPlaneParams,
                                  lightPos, lightWidth,
                                  ambientAlpha, spotAlpha, SK_ColorBLACK, flags);

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
        const SkScalar kAmbientAlpha = 0.1f;
        const SkScalar kSpotAlpha = 0.25f;

        SkPaint paint;
        paint.setAntiAlias(true);

        SkPoint3 lightPos = fLightPos;
        SkPoint3 zPlaneParams = SkPoint3::Make(0, 0, 0);

        paint.setColor(SK_ColorWHITE);
        canvas->translate(200, 90);
        zPlaneParams.fZ = SkTMax(1.0f, 2 + fZDelta);
        this->drawShadowedPath(canvas, fRRPath, zPlaneParams, paint, kAmbientAlpha,
                               lightPos, kLightWidth, kSpotAlpha);

        paint.setColor(SK_ColorRED);
        canvas->translate(250, 0);
        zPlaneParams.fZ = SkTMax(1.0f, 8 + fZDelta);
        this->drawShadowedPath(canvas, fRectPath, zPlaneParams, paint, kAmbientAlpha,
                               lightPos, kLightWidth, kSpotAlpha);

        paint.setColor(SK_ColorBLUE);
        canvas->translate(-250, 110);
        zPlaneParams.fZ = SkTMax(1.0f, 12 + fZDelta);
        this->drawShadowedPath(canvas, fCirclePath, zPlaneParams, paint, kAmbientAlpha,
                               lightPos, kLightWidth, 0.5f);

        paint.setColor(SK_ColorGREEN);
        canvas->translate(250, 0);
        zPlaneParams.fZ = SkTMax(1.0f, 64 + fZDelta);
        this->drawShadowedPath(canvas, fRRPath, zPlaneParams, paint, kAmbientAlpha,
                               lightPos, kLightWidth, kSpotAlpha);

        paint.setColor(SK_ColorYELLOW);
        canvas->translate(-250, 110);
        zPlaneParams.fZ = SkTMax(1.0f, 8 + fZDelta);
        this->drawShadowedPath(canvas, fFunkyRRPath, zPlaneParams, paint, kAmbientAlpha,
                               lightPos, kLightWidth, kSpotAlpha);

        paint.setColor(SK_ColorCYAN);
        canvas->translate(250, 0);
        zPlaneParams.fZ = SkTMax(1.0f, 16 + fZDelta);
        this->drawShadowedPath(canvas, fCubicPath, zPlaneParams, paint,
                               kAmbientAlpha, lightPos, kLightWidth, kSpotAlpha);

        // circular reveal
        SkPath tmpPath;
        SkPath tmpClipPath;
        tmpClipPath.addCircle(fAnimTranslate, 0, 60);
        Op(fSquareRRectPath, tmpClipPath, kIntersect_SkPathOp, &tmpPath);

        paint.setColor(SK_ColorMAGENTA);
        canvas->translate(-125, 60);
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
                                      SkScalarSin(-radians),
                                      SkTMax(1.0f, 16 + fZDelta) - SkScalarSin(-radians)*pivot.fY);
        this->drawShadowedPath(canvas, fWideRectPath, zPlaneParams, paint, .1f,
                               lightPos, kLightWidth, .5f);

        pivot = SkPoint::Make(fWideOvalPath.getBounds().width() / 2,
                              fWideOvalPath.getBounds().height() / 2);
        translate = SkPoint::Make(100, 600);
        view.restore();
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
    }

    bool onAnimate(const SkAnimTimer& timer) override {
        fAnimTranslate = timer.pingPong(30, 0, 200, -200);
        fAnimAngle = timer.pingPong(15, 0, 0, 20);

        return true;
    }

private:
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new ShadowsView; }
static SkViewRegister reg(MyFactory);
