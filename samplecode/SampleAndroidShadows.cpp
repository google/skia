
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
#include "SkGaussianEdgeShader.h"
#include "SkPath.h"
#include "SkPathOps.h"
#include "SkPoint3.h"
#include "SkShadowUtils.h"
#include "SkUtils.h"
#include "SkView.h"
#include "sk_tool_utils.h"

#define USE_SHADOW_UTILS

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
        , fUseAlt(true)
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

        fLightPos = SkPoint3::Make(-700, -700, 2800);
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

    static void GetOcclRect(const SkPath& path, SkRect* occlRect) {
        SkRect pathRect;
        SkRRect pathRRect;
        if (path.isOval(&pathRect)) {
            *occlRect = sk_tool_utils::compute_central_occluder(SkRRect::MakeOval(pathRect));
        } else if (path.isRRect(&pathRRect)) {
            *occlRect = sk_tool_utils::compute_central_occluder(pathRRect);
        } else if (path.isRect(occlRect)) {
            // the inverse transform for the spot shadow occluder doesn't always get us
            // back to exactly the same position, so deducting a little slop
            occlRect->inset(1, 1);
        } else {
            *occlRect = SkRect::MakeEmpty();
        }
    }

    void drawAmbientShadow(SkCanvas* canvas, const SkPath& path, SkScalar zValue, 
                           SkScalar ambientAlpha) {

        if (ambientAlpha <= 0) {
            return;
        }

        const SkScalar kHeightFactor = 1.f / 128.f;
        const SkScalar kGeomFactor = 64;

        SkScalar umbraAlpha = 1 / (1 + SkMaxScalar(zValue*kHeightFactor, 0));
        SkScalar radius = zValue*kHeightFactor*kGeomFactor;

        // occlude blur
        SkRect occlRect;
        GetOcclRect(path, &occlRect);
        sk_sp<SkMaskFilter> mf = SkBlurMaskFilter::Make(kNormal_SkBlurStyle,
                                                        SkBlurMask::ConvertRadiusToSigma(radius),
                                                        occlRect,
                                                        SkBlurMaskFilter::kNone_BlurFlag);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setMaskFilter(std::move(mf));
        paint.setColor(SkColorSetARGB(fIgnoreShadowAlpha
                                    ? 255
                                    : (unsigned char)(ambientAlpha*umbraAlpha*255.999f), 0, 0, 0));
        canvas->drawPath(path, paint);

        // draw occlusion rect
#if DRAW_OCCL_RECT
        SkPaint stroke;
        stroke.setStyle(SkPaint::kStroke_Style);
        stroke.setColor(SK_ColorBLUE);
        canvas->drawRect(occlRect, stroke);
#endif
    }

    void drawAmbientShadowAlt(SkCanvas* canvas, const SkPath& path, SkScalar zValue,
                              SkScalar ambientAlpha) {

        if (ambientAlpha <= 0) {
            return;
        }

        const SkScalar kHeightFactor = 1.f / 128.f;
        const SkScalar kGeomFactor = 64;

        SkScalar umbraAlpha = 1 / (1 + SkMaxScalar(zValue*kHeightFactor, 0));
        SkScalar radius = zValue*kHeightFactor*kGeomFactor;
        // distance to outer of edge of geometry from original shape edge
        SkScalar offset = radius*umbraAlpha;

        SkRect pathRect;
        SkRRect pathRRect;
        SkScalar scaleFactors[2];
        if (!canvas->getTotalMatrix().getMinMaxScales(scaleFactors)) {
            return;
        }
        if (scaleFactors[0] != scaleFactors[1] || radius*scaleFactors[0] >= 64 ||
            !((path.isOval(&pathRect) && pathRect.width() == pathRect.height()) ||
              (path.isRRect(&pathRRect) && pathRRect.allCornersCircular()) ||
              path.isRect(&pathRect))) {
            this->drawAmbientShadow(canvas, path, zValue, ambientAlpha);
            return;
        }

        // For all of these, we inset the offset rect by half the radius to get our stroke shape.
        SkScalar strokeOutset = offset - SK_ScalarHalf*radius;
        // Make sure we'll have a radius of at least 0.5 after xform
        if (strokeOutset*scaleFactors[0] < 0.5f) {
            strokeOutset = 0.5f / scaleFactors[0];
        }
        if (path.isOval(nullptr)) {
            pathRect.outset(strokeOutset, strokeOutset);
            pathRRect = SkRRect::MakeOval(pathRect);
        } else if (path.isRect(nullptr)) {
            pathRect.outset(strokeOutset, strokeOutset);
            pathRRect = SkRRect::MakeRectXY(pathRect, strokeOutset, strokeOutset);
        } else {
            pathRRect.outset(strokeOutset, strokeOutset);
        }

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        // we outset the stroke a little to cover up AA on the interior edge
        SkScalar pad = 0.5f;
        paint.setStrokeWidth(radius + 2*pad);
        // handle scale of radius and pad due to CTM
        radius *= scaleFactors[0];
        pad *= scaleFactors[0];
        SkASSERT(radius < 16384);
        SkASSERT(pad < 64);
        // Convert radius to 14.2 fixed point and place in the R & G components.
        // Convert pad to 6.2 fixed point and place in the B component.
        uint16_t iRadius = (uint16_t)(radius*4.0f);
        unsigned char alpha = (unsigned char)(ambientAlpha*255.999f);
        paint.setColor(SkColorSetARGB(fIgnoreShadowAlpha ? 255 : alpha,
                                      iRadius >> 8, iRadius & 0xff,
                                      (unsigned char)(4.0f*pad)));

        paint.setColorFilter(SkColorFilter::MakeModeFilter(SK_ColorBLACK, SkBlendMode::kModulate));
        paint.setShader(SkGaussianEdgeShader::Make());
        canvas->drawRRect(pathRRect, paint);
    }

    void drawSpotShadow(SkCanvas* canvas, const SkPath& path, SkScalar zValue,
                        SkPoint3 lightPos, SkScalar lightWidth, SkScalar spotAlpha) {
        if (spotAlpha <= 0) {
            return;
        }

        SkScalar zRatio = zValue / (lightPos.fZ - zValue);
        if (zRatio < 0.0f) {
            zRatio = 0.0f;
        } else if (zRatio > 0.95f) {
            zRatio = 0.95f;
        }
        SkScalar blurRadius = lightWidth*zRatio;

        // compute the transformation params
        SkPoint center = SkPoint::Make(path.getBounds().centerX(), path.getBounds().centerY());
        SkMatrix ctmInverse;
        if (!canvas->getTotalMatrix().invert(&ctmInverse)) {
            return;
        }
        SkPoint lightPos2D = SkPoint::Make(lightPos.fX, lightPos.fY);
        ctmInverse.mapPoints(&lightPos2D, 1);
        SkPoint offset = SkPoint::Make(zRatio*(center.fX - lightPos2D.fX),
                                       zRatio*(center.fY - lightPos2D.fY));
        SkScalar scale = lightPos.fZ / (lightPos.fZ - zValue);

        SkAutoCanvasRestore acr(canvas, true);

        sk_sp<SkMaskFilter> mf = SkBlurMaskFilter::Make(kNormal_SkBlurStyle,
                                                        SkBlurMask::ConvertRadiusToSigma(blurRadius),
                                                        SkBlurMaskFilter::kNone_BlurFlag);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setMaskFilter(std::move(mf));
        paint.setColor(SkColorSetARGB(fIgnoreShadowAlpha
                                                ? 255 
                                                : (unsigned char)(spotAlpha*255.999f), 0, 0, 0));

        // apply transformation to shadow
        canvas->scale(scale, scale);
        canvas->translate(offset.fX, offset.fY);
        canvas->drawPath(path, paint);
    }

    void drawSpotShadowAlt(SkCanvas* canvas, const SkPath& path, SkScalar zValue,
                        SkPoint3 lightPos, SkScalar lightWidth, SkScalar spotAlpha) {
        if (spotAlpha <= 0) {
            return;
        }

        SkScalar zRatio = zValue / (lightPos.fZ - zValue);
        if (zRatio < 0.0f) {
            zRatio = 0.0f;
        } else if (zRatio > 0.95f) {
            zRatio = 0.95f;
        }
        SkScalar radius = 2.0f*lightWidth*zRatio;

        SkRect pathRect;
        SkRRect pathRRect;
        SkScalar scaleFactors[2];
        if (!canvas->getTotalMatrix().getMinMaxScales(scaleFactors)) {
            return;
        }
        if (scaleFactors[0] != scaleFactors[1] || radius*scaleFactors[0] >= 16384 ||
            !((path.isOval(&pathRect) && pathRect.width() == pathRect.height()) ||
              (path.isRRect(&pathRRect) && pathRRect.allCornersCircular()) ||
              path.isRect(&pathRect))) {
            this->drawSpotShadow(canvas, path, zValue, lightPos, lightWidth, spotAlpha);
            return;
        }

        // For all of these, we need to ensure we have a rrect with radius >= 0.5f in device space
        const SkScalar minRadius = SK_ScalarHalf/scaleFactors[0];
        if (path.isOval(nullptr)) {
            pathRRect = SkRRect::MakeOval(pathRect);
        } else if (path.isRect(nullptr)) {
            pathRRect = SkRRect::MakeRectXY(pathRect, minRadius, minRadius);
        } else {
            if (pathRRect.getSimpleRadii().fX < minRadius) {
                pathRRect.setRectXY(pathRRect.rect(), minRadius, minRadius);
            }
        }

        // compute the scale and translation for the shadow
        SkScalar scale = lightPos.fZ / (lightPos.fZ - zValue);
        SkRRect shadowRRect;
        pathRRect.transform(SkMatrix::MakeScale(scale, scale), &shadowRRect);
        SkPoint center = SkPoint::Make(shadowRRect.rect().centerX(), shadowRRect.rect().centerY());
        SkMatrix ctmInverse;
        if (!canvas->getTotalMatrix().invert(&ctmInverse)) {
            return;
        }
        SkPoint lightPos2D = SkPoint::Make(lightPos.fX, lightPos.fY);
        ctmInverse.mapPoints(&lightPos2D, 1);
        SkPoint offset = SkPoint::Make(zRatio*(center.fX - lightPos2D.fX),
                                       zRatio*(center.fY - lightPos2D.fY));
        SkAutoCanvasRestore acr(canvas, true);

        SkPaint paint;
        paint.setAntiAlias(true);
        // We want to extend the stroked area in so that it meets up with the caster
        // geometry. The stroked geometry will, by definition already be inset half the
        // stroke width but we also have to account for the scaling.
        // We also add 1/2 to cover up AA on the interior edge.
        SkScalar scaleOffset = (scale - 1.0f) * SkTMax(SkTMax(SkTAbs(pathRect.fLeft),
                                                              SkTAbs(pathRect.fRight)),
                                                       SkTMax(SkTAbs(pathRect.fTop),
                                                              SkTAbs(pathRect.fBottom)));
        SkScalar insetAmount = offset.length() - (0.5f * radius) + scaleOffset + 0.5f;

        // compute area
        SkScalar strokeWidth = radius + insetAmount;
        SkScalar strokedArea = 2.0f*strokeWidth*(shadowRRect.width() + shadowRRect.height());
        SkScalar filledArea = (shadowRRect.height() + radius)*(shadowRRect.width() + radius);
        // If the area of the stroked geometry is larger than the fill geometry, or
        // if our pad is too big to convert to 6.2 fixed point, just fill it.
        if (strokedArea > filledArea) {
            paint.setStyle(SkPaint::kStrokeAndFill_Style);
            paint.setStrokeWidth(radius);
        } else {
            // Since we can't have unequal strokes, inset the shadow rect so the inner
            // and outer edges of the stroke will land where we want.
            SkRect insetRect = shadowRRect.rect().makeInset(insetAmount/2.0f, insetAmount/2.0f);
            SkScalar insetRad = SkTMax(shadowRRect.getSimpleRadii().fX - insetAmount/2.0f,
                                       minRadius);

            shadowRRect = SkRRect::MakeRectXY(insetRect, insetRad, insetRad);
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setStrokeWidth(strokeWidth);
        }
        paint.setColorFilter(SkColorFilter::MakeModeFilter(SK_ColorBLACK, SkBlendMode::kModulate));
        paint.setShader(SkGaussianEdgeShader::Make());
        // handle scale of radius due to CTM
        radius *= scaleFactors[0];
        // don't need to scale pad as it was computed from the transformed offset
        SkASSERT(radius < 16384);
        SkScalar pad = 0;
        SkASSERT(pad < 64);
        // Convert radius to 14.2 fixed point and place in the R & G components.
        // Convert pad to 6.2 fixed point and place in the B component.
        uint16_t iRadius = (uint16_t)(radius*4.0f);
        unsigned char alpha = (unsigned char)(spotAlpha*255.999f);
        paint.setColor(SkColorSetARGB(fIgnoreShadowAlpha ? 255 : alpha,
                                      iRadius >> 8, iRadius & 0xff,
                                      (unsigned char)(4.0f*pad)));

        // apply transformation to shadow
        canvas->translate(offset.fX, offset.fY);
        canvas->drawRRect(shadowRRect, paint);
    }

    void drawShadowedPath(SkCanvas* canvas, const SkPath& path,
                          std::function<SkScalar(SkScalar, SkScalar)> zFunc,
                          const SkPaint& paint, SkScalar ambientAlpha,
                          const SkPoint3& lightPos, SkScalar lightWidth, SkScalar spotAlpha) {
#ifdef USE_SHADOW_UTILS
        SkScalar zValue = zFunc(0, 0);
        if (fUseAlt) {
            if (fShowAmbient) {
                this->drawAmbientShadowAlt(canvas, path, zValue, ambientAlpha);
            }
            if (fShowSpot) {
                this->drawSpotShadowAlt(canvas, path, zValue, lightPos, lightWidth, spotAlpha);
            }
        } else {
            if (!fShowAmbient) {
                ambientAlpha = 0;
            }
            if (!fShowSpot) {
                spotAlpha = 0;
            }

            //SkShadowUtils::DrawShadow(canvas, path,
            //                          zValue,
            //                          lightPos, lightWidth,
            //                          ambientAlpha, spotAlpha, SK_ColorBLACK);
            SkShadowUtils::DrawUncachedShadow(canvas, path, zFunc,
                                              lightPos, lightWidth,
                                              ambientAlpha, spotAlpha, SK_ColorBLACK);
        }
#else
        if (fShowAmbient) {
            if (fUseAlt) {
                this->drawAmbientShadowAlt(canvas, path, zValue, ambientAlpha);
            } else {
                this->drawAmbientShadow(canvas, path, zValue, ambientAlpha);
            }
        }
        if (fShowSpot) {
            if (fUseAlt) {
                this->drawSpotShadowAlt(canvas, path, zValue, lightPos, lightWidth, spotAlpha);
            } else {
                this->drawSpotShadow(canvas, path, zValue, lightPos, lightWidth, spotAlpha);
            }
        }
#endif

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
        const SkScalar kLightWidth = 2800;
        const SkScalar kAmbientAlpha = 0.25f;
        const SkScalar kSpotAlpha = 0.25f;

        SkPaint paint;
        paint.setAntiAlias(true);

        SkPoint3 lightPos = fLightPos;

        paint.setColor(SK_ColorWHITE);
        canvas->translate(200, 90);
        lightPos.fX += 200;
        lightPos.fY += 90;
        SkScalar zValue = SkTMax(1.0f, 2 + fZDelta);
        std::function<SkScalar(SkScalar, SkScalar)> zFunc = 
            [zValue](SkScalar, SkScalar) { return zValue; };
        this->drawShadowedPath(canvas, fRRPath, zFunc, paint, kAmbientAlpha,
                               lightPos, kLightWidth, kSpotAlpha);

        paint.setColor(SK_ColorRED);
        canvas->translate(250, 0);
        lightPos.fX += 250;
        zValue = SkTMax(1.0f, 4 + fZDelta);
        zFunc = [zValue](SkScalar, SkScalar) { return zValue; };
        this->drawShadowedPath(canvas, fRectPath, zFunc, paint, kAmbientAlpha,
                               lightPos, kLightWidth, kSpotAlpha);

        paint.setColor(SK_ColorBLUE);
        canvas->translate(-250, 110);
        lightPos.fX -= 250;
        lightPos.fY += 110;
        zValue = SkTMax(1.0f, 8 + fZDelta);
        zFunc = [zValue](SkScalar, SkScalar) { return zValue; };
        this->drawShadowedPath(canvas, fCirclePath, zFunc, paint, kAmbientAlpha,
                               lightPos, kLightWidth, 0.5f);

        paint.setColor(SK_ColorGREEN);
        canvas->translate(250, 0);
        lightPos.fX += 250;
        zValue = SkTMax(1.0f, 64 + fZDelta);
        zFunc = [zValue](SkScalar, SkScalar) { return zValue; };
        this->drawShadowedPath(canvas, fRRPath, zFunc, paint, kAmbientAlpha,
                               lightPos, kLightWidth, kSpotAlpha);

        paint.setColor(SK_ColorYELLOW);
        canvas->translate(-250, 110);
        lightPos.fX -= 250;
        lightPos.fY += 110;
        zValue = SkTMax(1.0f, 8 + fZDelta);
        zFunc = [zValue](SkScalar, SkScalar) { return zValue; };
        this->drawShadowedPath(canvas, fFunkyRRPath, zFunc, paint, kAmbientAlpha,
                               lightPos, kLightWidth, kSpotAlpha);

        paint.setColor(SK_ColorCYAN);
        canvas->translate(250, 0);
        lightPos.fX += 250;
        zValue = SkTMax(1.0f, 16 + fZDelta);
        zFunc = [zValue](SkScalar, SkScalar) { return zValue; };
        this->drawShadowedPath(canvas, fCubicPath, zFunc, paint,
                               kAmbientAlpha, lightPos, kLightWidth, kSpotAlpha);

        // circular reveal
        SkPath tmpPath;
        SkPath tmpClipPath;
        tmpClipPath.addCircle(fAnimTranslate, 0, 60);
        Op(fSquareRRectPath, tmpClipPath, kIntersect_SkPathOp, &tmpPath);

        paint.setColor(SK_ColorMAGENTA);
        canvas->translate(-125, 60);
        lightPos.fX -= 125;
        lightPos.fY += 60;
        zValue = SkTMax(1.0f, 32 + fZDelta);
        zFunc = [zValue](SkScalar, SkScalar) { return zValue; };
        this->drawShadowedPath(canvas, tmpPath, zFunc, paint, .1f,
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
        lightPos = fLightPos;
        lightPos.fX += pivot.fX + translate.fX;
        lightPos.fY += pivot.fY + translate.fY;
        zValue = SkTMax(1.0f, 16 + fZDelta);
        SkScalar radians = SkDegreesToRadians(fAnimAngle);
        zFunc = [zValue, pivot, radians](SkScalar x, SkScalar y) {
            return SkScalarSin(-radians)*y +
                   zValue - SkScalarSin(-radians)*pivot.fY;
        };
        this->drawShadowedPath(canvas, fWideRectPath, zFunc, paint, .1f,
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
        lightPos = fLightPos;
        lightPos.fX += pivot.fX + translate.fX;
        lightPos.fY += pivot.fY + translate.fY;
        zValue = SkTMax(1.0f, 32 + fZDelta);
        zFunc = [zValue, pivot, radians](SkScalar x, SkScalar y) {
            return -SkScalarSin(radians)*x +
                zValue + SkScalarSin(radians)*pivot.fX;
        };
        this->drawShadowedPath(canvas, fWideOvalPath, zFunc, paint, .1f,
                               lightPos, kLightWidth, .5f);
    }

    bool onAnimate(const SkAnimTimer& timer) override {
        fAnimTranslate = timer.pingPong(30, 0, 200, -200);
        fAnimAngle = timer.pingPong(15, 0, 0, 20);

        return true;
    }

protected:
    SkView::Click* onFindClickHandler(SkScalar x, SkScalar y, unsigned modi) override {
        return new SkView::Click(this);
    }

    bool onClick(Click *click) override {
        SkScalar x = click->fCurr.fX;
        SkScalar y = click->fCurr.fY;

        SkScalar dx = x - click->fPrev.fX;
        SkScalar dy = y - click->fPrev.fY;

        if (dx != 0 || dy != 0) {
            fLightPos.fX += dx;
            fLightPos.fY += dy;
            this->inval(nullptr);
        }

        return true;
    }

private:
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new ShadowsView; }
static SkViewRegister reg(MyFactory);
