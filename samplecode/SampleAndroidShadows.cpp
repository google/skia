
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
#include "SkOffsetPolygon.h"
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

        fNotchPath.moveTo(-75, -100);
        fNotchPath.lineTo(75, -100);
        fNotchPath.lineTo(75, 100);
        fNotchPath.arcTo(SkRect::MakeLTRB(-20, 80, 20, 120), 0, -180, false);
        fNotchPath.lineTo(-75, 100);

        fTabPath.moveTo(-75, -100);
        fTabPath.lineTo(75, -100);
        fTabPath.lineTo(75, 100);
        fTabPath.arcTo(SkRect::MakeLTRB(-20, 80, 20, 120), 0, 180, false);
        fTabPath.lineTo(-75, 100);

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
        const SkScalar kAmbientAlpha = 0.1f;
        const SkScalar kSpotAlpha = 0.25f;

        SkPaint paint;
        paint.setAntiAlias(true);

        SkPoint3 lightPos = fLightPos;
        SkPoint3 zPlaneParams = SkPoint3::Make(0, 0, 0);

        paint.setColor(SK_ColorWHITE);
        canvas->translate(200, 90);
        //zPlaneParams.fZ = SkTMax(1.0f, 2 + fZDelta);
        //this->drawShadowedPath(canvas, fRRPath, zPlaneParams, paint, kAmbientAlpha,
        //                       lightPos, kLightWidth, kSpotAlpha);

        paint.setColor(SK_ColorRED);
        canvas->translate(250, 0);
        zPlaneParams.fZ = SkTMax(1.0f, 8 + fZDelta);
        //this->drawShadowedPath(canvas, fRectPath, zPlaneParams, paint, kAmbientAlpha,
        //                       lightPos, kLightWidth, kSpotAlpha);

        //paint.setColor(SK_ColorBLUE);
        //canvas->translate(-250, 110);
        //zPlaneParams.fZ = SkTMax(1.0f, 12 + fZDelta);
        //this->drawShadowedPath(canvas, fCirclePath, zPlaneParams, paint, kAmbientAlpha,
        //                       lightPos, kLightWidth, 0.5f);

        //paint.setColor(SK_ColorGREEN);
        //canvas->translate(250, 0);
        //zPlaneParams.fZ = SkTMax(1.0f, 64 + fZDelta);
        //this->drawShadowedPath(canvas, fRRPath, zPlaneParams, paint, kAmbientAlpha,
        //                       lightPos, kLightWidth, kSpotAlpha);

        //paint.setColor(SK_ColorYELLOW);
        //canvas->translate(-250, 110);
        //zPlaneParams.fZ = SkTMax(1.0f, 8 + fZDelta);
        //this->drawShadowedPath(canvas, fFunkyRRPath, zPlaneParams, paint, kAmbientAlpha,
        //                       lightPos, kLightWidth, kSpotAlpha);

        //paint.setColor(SK_ColorCYAN);
        //canvas->translate(250, 0);
        //zPlaneParams.fZ = SkTMax(1.0f, 16 + fZDelta);
        //this->drawShadowedPath(canvas, fCubicPath, zPlaneParams, paint,
        //                       kAmbientAlpha, lightPos, kLightWidth, kSpotAlpha);

        //////////////////////////////////////////////////////////////////////////////////////////

        paint.setColor(SK_ColorWHITE);
        canvas->translate(-250, 110);
        zPlaneParams.fZ = SkTMax(1.0f, 8 + fZDelta);
        this->drawShadowedPath(canvas, fStarPath, zPlaneParams, paint,
                               kAmbientAlpha, lightPos, kLightWidth, kSpotAlpha);

        SkPoint starVerts[] = {
            { 0.0f, -50.0f },
            { 14.43f, -25.0f },
            { 43.30f, -25.0f },
            { 28.86f, 0.0f },
            { 43.30f, 25.0f },
            { 14.43f, 25.0f },
            { 0.0f, 50.0f },
            { -14.43f, 25.0f },
            { -43.30f, 25.0f },
            { -28.86f, 0.0f },
            { -43.30f, -25.0f },
            { -14.43f, -25.0f },
        };
        paint.setColor(SK_ColorBLACK);
        for (int i = 0; i < SK_ARRAY_COUNT(starVerts) - 1; ++i) {
            canvas->drawLine(starVerts[i], starVerts[i + 1], paint);
        }
        canvas->drawLine(starVerts[SK_ARRAY_COUNT(starVerts) - 1], starVerts[0], paint);

        SkTDArray<SkPoint> offsetPoly;
        SkOffsetPolygon(starVerts, SK_ARRAY_COUNT(starVerts), -22.0f, &offsetPoly);
        for (int i = 0; i < offsetPoly.count() - 1; ++i) {
            canvas->drawLine(offsetPoly[i], offsetPoly[i + 1], paint);
        }
        canvas->drawLine(offsetPoly[offsetPoly.count() - 1], offsetPoly[0], paint);

        SkOffsetPolygon(starVerts, SK_ARRAY_COUNT(starVerts), 22.0f, &offsetPoly);
        for (int i = 0; i < offsetPoly.count() - 1; ++i) {
            canvas->drawLine(offsetPoly[i], offsetPoly[i + 1], paint);
        }
        canvas->drawLine(offsetPoly[offsetPoly.count() - 1], offsetPoly[0], paint);

        //////////////////////////////////////////////////////////////////////////////////////////

        paint.setColor(SK_ColorWHITE);
        canvas->translate(150, 0);
        zPlaneParams.fZ = SkTMax(1.0f, 2 + fZDelta);
        this->drawShadowedPath(canvas, fNotchPath, zPlaneParams, paint,
                               kAmbientAlpha, lightPos, kLightWidth, kSpotAlpha);
        SkPoint notchVerts[] = {
            {-75, -100},
            {75, -100},
            {75, 100},
            {20, 100},
            {19.6157f,100.f-3.9018f},
            {18.4776f,100.f - 7.6537f},
            {16.6294f,100.f - 11.1114f},
            {14.1421f,100.f - 14.1421f},
            { 11.1114f,100.f - 16.6294f},
            { 7.6537f,100.f - 18.4776f},
            { 3.9018f,100.f - 19.6157f},
            { 0, 80.f},
            { -3.9018f,100 - 19.6157f },
            { -7.6537f,100 - 18.4776f },
            { -11.1114f,100 - 16.6294f },
            { -14.1421f,100 - 14.1421f },
            { -16.6294f,100 - 11.1114f },
            { -18.4776f,100 - 7.6537f },
            { -19.6157f,100 - 3.9018f },
            { -20, 100 },
            {-75, 100}
        };
        paint.setColor(SK_ColorBLACK);
        for (int i = 0; i < SK_ARRAY_COUNT(notchVerts) - 1; ++i) {
            canvas->drawLine(notchVerts[i], notchVerts[i + 1], paint);
        }
        canvas->drawLine(notchVerts[SK_ARRAY_COUNT(notchVerts) - 1], notchVerts[0], paint);

        SkOffsetPolygon(notchVerts, SK_ARRAY_COUNT(notchVerts), -22.0f, &offsetPoly);
        for (int i = 0; i < offsetPoly.count() - 1; ++i) {
            canvas->drawLine(offsetPoly[i], offsetPoly[i + 1], paint);
        }
        canvas->drawLine(offsetPoly[offsetPoly.count() - 1], offsetPoly[0], paint);

        SkOffsetPolygon(notchVerts, SK_ARRAY_COUNT(notchVerts), 22.0f, &offsetPoly);
        for (int i = 0; i < offsetPoly.count() - 1; ++i) {
            canvas->drawLine(offsetPoly[i], offsetPoly[i + 1], paint);
        }
        canvas->drawLine(offsetPoly[offsetPoly.count() - 1], offsetPoly[0], paint);

        //////////////////////////////////////////////////////////////////////////////////////////

        paint.setColor(SK_ColorWHITE);
        canvas->translate(200, 0);
        zPlaneParams.fZ = SkTMax(1.0f, 2 + fZDelta);
        this->drawShadowedPath(canvas, fTabPath, zPlaneParams, paint,
                               kAmbientAlpha, lightPos, kLightWidth, kSpotAlpha);

        SkPoint tabVerts[] = {
            { -75, -100 },
            { 75, -100 },
            { 75, 100 },
            { 20, 100 },
            { 19.6157f,100.f + 3.9018f },
            { 18.4776f,100.f + 7.6537f },
            { 16.6294f,100.f + 11.1114f },
            { 14.1421f,100.f + 14.1421f },
            { 11.1114f,100.f + 16.6294f },
            { 7.6537f,100.f + 18.4776f },
            { 3.9018f,100.f + 19.6157f },
            { 0, 120.f },
            { -3.9018f,100 + 19.6157f },
            { -7.6537f,100 + 18.4776f },
            { -11.1114f,100 + 16.6294f },
            { -14.1421f,100 + 14.1421f },
            { -16.6294f,100 + 11.1114f },
            { -18.4776f,100 + 7.6537f },
            { -19.6157f,100 + 3.9018f },
            { -20, 100 },
            { -75, 100 }
        };
        paint.setColor(SK_ColorBLACK);
        for (int i = 0; i < SK_ARRAY_COUNT(tabVerts) - 1; ++i) {
            canvas->drawLine(tabVerts[i], tabVerts[i + 1], paint);
        }
        canvas->drawLine(tabVerts[SK_ARRAY_COUNT(tabVerts) - 1], tabVerts[0], paint);

        SkOffsetPolygon(tabVerts, SK_ARRAY_COUNT(tabVerts), -22.0f, &offsetPoly);
        for (int i = 0; i < offsetPoly.count() - 1; ++i) {
            canvas->drawLine(offsetPoly[i], offsetPoly[i + 1], paint);
        }
        canvas->drawLine(offsetPoly[offsetPoly.count() - 1], offsetPoly[0], paint);

        SkOffsetPolygon(tabVerts, SK_ARRAY_COUNT(tabVerts), 22.0f, &offsetPoly);
        for (int i = 0; i < offsetPoly.count() - 1; ++i) {
            canvas->drawLine(offsetPoly[i], offsetPoly[i + 1], paint);
        }
        canvas->drawLine(offsetPoly[offsetPoly.count() - 1], offsetPoly[0], paint);

#if 0
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
#endif
    }

    bool onAnimate(const SkAnimTimer& timer) override {
        return false;

        //fAnimTranslate = timer.pingPong(30, 0, 200, -200);
        //fAnimAngle = timer.pingPong(15, 0, 0, 20);
        //if (fDoAlphaAnimation) {
        //    fAnimAlpha = timer.pingPong(5, 0, 1, 0);
        //}
        //return true;
    }

private:
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new ShadowsView; }
static SkViewRegister reg(MyFactory);
