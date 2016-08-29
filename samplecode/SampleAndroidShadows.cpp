
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SampleCode.h"
#include "SkBlurMask.h"
#include "SkBlurMaskFilter.h"
#include "SkCanvas.h"
#include "SkGaussianEdgeShader.h"
#include "SkPath.h"
#include "SkPoint3.h"
#include "SkUtils.h"
#include "SkView.h"
#include "sk_tool_utils.h"

////////////////////////////////////////////////////////////////////////////

class ShadowsView : public SampleView {
    SkPath    fRectPath;
    SkPath    fRRPath;
    SkPath    fCirclePath;
    SkPoint3  fLightPos;

    bool      fShowAmbient;
    bool      fShowSpot;
    bool      fUseAlt;
    bool      fShowObject;

public:
    ShadowsView() 
        : fShowAmbient(true)
        , fShowSpot(true)
        , fUseAlt(true)
        , fShowObject(true) {}

protected:
    void onOnceBeforeDraw() override {
        fCirclePath.addCircle(0, 0, 50);
        fRectPath.addRect(SkRect::MakeXYWH(-100, -50, 200, 100));
        fRRPath.addRRect(SkRRect::MakeRectXY(SkRect::MakeXYWH(-100, -50, 200, 100), 4, 4));
        fLightPos = SkPoint3::Make(-2, -2, 6);
    }

    // overrides from SkEventSink
    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "AndroidShadows");
            return true;
        }

        SkUnichar uni;
        if (SampleCode::CharQ(*evt, &uni)) {
            switch (uni) {
                case 'B':
                    fShowAmbient = !fShowAmbient;
                    break;
                case 'S':
                    fShowSpot = !fShowSpot;
                    break;
                case 'T':
                    fUseAlt = !fUseAlt;
                    break;
                case 'O':
                    fShowObject = !fShowObject;
                    break;
                case '>':
                    fLightPos.fZ += 10;
                    break;
                case '<':
                    fLightPos.fZ -= 10;
                    break;
                default:
                    break;
            }
            this->inval(nullptr);
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
        paint.setColor(SkColorSetARGB((unsigned char)(ambientAlpha*umbraAlpha*255.999f), 0, 0, 0));
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

        SkRect pathRect;
        SkRRect pathRRect;
        if (radius >= 64 ||
            !((path.isOval(&pathRect) && pathRect.width() == pathRect.height()) ||
              (path.isRRect(&pathRRect) && pathRRect.allCornersCircular()) ||
              path.isRect(&pathRect))) {
            this->drawAmbientShadow(canvas, path, zValue, ambientAlpha);
            return;
        }

        // For all of these, we outset the rect by half the radius to get our stroke shape.
        SkScalar halfRadius = SK_ScalarHalf*radius;
        if (path.isOval(nullptr)) {
            pathRect.outset(halfRadius, halfRadius);
            pathRRect = SkRRect::MakeOval(pathRect);
        } else if (path.isRect(nullptr)) {
            pathRect.outset(halfRadius, halfRadius);
            pathRRect = SkRRect::MakeRectXY(pathRect, halfRadius, halfRadius);
        } else {
            pathRRect.outset(halfRadius, halfRadius);
        }

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        // we outset the stroke a little to cover up AA on the interior edge
        paint.setStrokeWidth(radius + 1);
        // handle scale of radius due to CTM
        SkScalar maxScale = canvas->getTotalMatrix().getMaxScale();
        radius *= maxScale;
        unsigned char gray = (unsigned char)(ambientAlpha*umbraAlpha*255.999f);
        SkASSERT(radius < 64);
        // Convert radius to 6.2 fixed point and place in the G component.
        paint.setColor(SkColorSetARGB(1, gray, (unsigned char)(4.0f*radius), 0));

        sk_sp<SkShader> gaussShader = SkGaussianEdgeShader::Make();
        paint.setShader(gaussShader);
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
        SkScalar radius = lightWidth*zRatio;

        // compute the transformation params
        SkPoint center = SkPoint::Make(path.getBounds().centerX(), path.getBounds().centerY());
        canvas->getTotalMatrix().mapPoints(&center, 1);
        SkPoint offset = SkPoint::Make(-zRatio*(lightPos.fX - center.fX), 
                                       -zRatio*(lightPos.fY - center.fY));
        SkScalar scale = lightPos.fZ / (lightPos.fZ - zValue);
        if (scale < 1.0f) {
            scale = 1.0f;
        } else if (scale > 1024.f) {
            scale = 1024.f;
        }

        SkAutoCanvasRestore acr(canvas, true);

        SkRect occlRect;
        GetOcclRect(path, &occlRect);
        // apply inverse transform
        occlRect.offset(-offset);
#if 0
        // It looks like the scale may be invalid
        SkScalar scale = lightPos.fZ / (lightPos.fZ - zValue);
        if (scale < 1.0f) {
            scale = 1.0f;
        } else if (scale > 1024.f) {
            scale = 1024.f;
        }
        occlRect.fLeft /= scale;
        occlRect.fRight /= scale;
        occlRect.fTop /= scale;
        occlRect.fBottom /= scale;
#endif
        sk_sp<SkMaskFilter> mf = SkBlurMaskFilter::Make(kNormal_SkBlurStyle,
                                                        SkBlurMask::ConvertRadiusToSigma(radius),
                                                        occlRect,
                                                        SkBlurMaskFilter::kNone_BlurFlag);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setMaskFilter(std::move(mf));
        paint.setColor(SkColorSetARGB((unsigned char)(spotAlpha*255.999f), 0, 0, 0));

        // apply transformation to shadow
        canvas->translate(offset.fX, offset.fY);
#if 0
        // It looks like the scale may be invalid
        canvas->scale(scale, scale);
#endif
        canvas->drawPath(path, paint);

        // draw occlusion rect
#if DRAW_OCCL_RECT
        SkPaint stroke;
        stroke.setStyle(SkPaint::kStroke_Style);
        stroke.setColor(SK_ColorRED);
        canvas->drawRect(occlRect, stroke)
#endif
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
        SkScalar radius = lightWidth*zRatio;

        SkRect pathRect;
        SkRRect pathRRect;
        if (radius >= 64 ||
            !((path.isOval(&pathRect) && pathRect.width() == pathRect.height()) ||
              (path.isRRect(&pathRRect) && pathRRect.allCornersCircular()) ||
              path.isRect(&pathRect))) {
            this->drawSpotShadow(canvas, path, zValue, lightPos, lightWidth, spotAlpha);
            return;
        }

        // For all of these, we outset the rect by half the radius to get our stroke shape.
        SkScalar halfRadius = SK_ScalarHalf*radius;
        if (path.isOval(nullptr)) {
            pathRect.outset(halfRadius, halfRadius);
            pathRRect = SkRRect::MakeOval(pathRect);
        } else if (path.isRect(nullptr)) {
            pathRect.outset(halfRadius, halfRadius);
            pathRRect = SkRRect::MakeRectXY(pathRect, halfRadius, halfRadius);
        } else {
            pathRRect.outset(halfRadius, halfRadius);
        }

        // compute the transformation params
        SkPoint center = SkPoint::Make(path.getBounds().centerX(), path.getBounds().centerY());
        canvas->getTotalMatrix().mapPoints(&center, 1);
        SkPoint offset = SkPoint::Make(-zRatio*(lightPos.fX - center.fX),
                                       -zRatio*(lightPos.fY - center.fY));
        SkAutoCanvasRestore acr(canvas, true);

        SkPaint paint;
        paint.setAntiAlias(true);
        // We outset the stroke by the length of the translation so the shadow extends to
        // the edge of the shape. We also add 1/2 to cover up AA on the interior edge.
        SkScalar pad = offset.length() + 0.5f;
        // compute area
        SkScalar strokeWidth = radius + 2.0f*pad;
        SkScalar strokedArea = 2.0f*strokeWidth*(pathRRect.width() + pathRRect.height());
        SkScalar filledArea = (pathRRect.height() + radius)*(pathRRect.width() + radius);
        // If the area of the stroked geometry is larger than the fill geometry, or
        // if our pad is too big to convert to 6.2 fixed point, just fill it.
        if (strokedArea > filledArea || pad >= 64) {
            pad = 0;
            paint.setStyle(SkPaint::kStrokeAndFill_Style);
            paint.setStrokeWidth(radius);
        } else {
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setStrokeWidth(strokeWidth);
        }
        sk_sp<SkShader> gaussShader = SkGaussianEdgeShader::Make();
        paint.setShader(gaussShader);
        // handle scale of radius due to CTM
        SkScalar maxScale = canvas->getTotalMatrix().getMaxScale();
        radius *= maxScale;
        unsigned char gray = (unsigned char)(spotAlpha*255.999f);
        SkASSERT(radius < 64);
        SkASSERT(pad < 64);
        // Convert radius and pad to 6.2 fixed point and place in the G & B components.
        paint.setColor(SkColorSetARGB(1, gray, (unsigned char)(radius*4.0f),
                                     (unsigned char)(pad*4.0f)));

        // apply transformation to shadow
        canvas->translate(offset.fX, offset.fY);
#if 0
        // It looks like the scale may be invalid
        SkScalar scale = lightPos.fZ / (lightPos.fZ - zValue);
        if (scale < 1.0f) {
            scale = 1.0f;
        } else if (scale > 1024.f) {
            scale = 1024.f;
        }
        canvas->scale(scale, scale);
#endif
        canvas->drawRRect(pathRRect, paint);
    }

    void drawShadowedPath(SkCanvas* canvas, const SkPath& path, SkScalar zValue,
                          const SkPaint& paint, SkScalar ambientAlpha,
                          const SkPoint3& lightPos, SkScalar lightWidth, SkScalar spotAlpha) {
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
        if (fShowObject) {
            canvas->drawPath(path, paint);
        }
    }

    void onDrawContent(SkCanvas* canvas) override {
        this->drawBG(canvas);
        const SkScalar kLightWidth = 3;
        const SkScalar kAmbientAlpha = 0.25f;
        const SkScalar kSpotAlpha = 0.25f;

        SkPaint paint;
        paint.setAntiAlias(true);

        SkPoint3 lightPos = fLightPos;

        paint.setColor(SK_ColorWHITE);
        canvas->translate(200, 90);
        lightPos.fX += 200;
        lightPos.fY += 90;
        this->drawShadowedPath(canvas, fRectPath, 5, paint, kAmbientAlpha, 
                               lightPos, kLightWidth, kSpotAlpha);

        paint.setColor(SK_ColorRED);
        canvas->translate(250, 0);
        lightPos.fX += 250;
        this->drawShadowedPath(canvas, fRRPath, 5, paint, kAmbientAlpha,
                               lightPos, kLightWidth, kSpotAlpha);

        paint.setColor(SK_ColorBLUE);
        canvas->translate(-250, 110);
        lightPos.fX -= 250;
        lightPos.fY += 110;
        this->drawShadowedPath(canvas, fCirclePath, 5, paint, 0.0f,
                               lightPos, kLightWidth, 0.5f);

        paint.setColor(SK_ColorGREEN);
        canvas->translate(250, 0);
        lightPos.fX += 250;
        this->drawShadowedPath(canvas, fRRPath, 5, paint, kAmbientAlpha,
                               lightPos, kLightWidth, kSpotAlpha);
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
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new ShadowsView; }
static SkViewRegister reg(MyFactory);
