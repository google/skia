
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
    bool      fUseAltAmbient;
    bool      fShowSpot;
    bool      fShowObject;

public:
    ShadowsView() 
        : fShowAmbient(true)
        , fUseAltAmbient(true)
        , fShowSpot(true)
        , fShowObject(true) {}

protected:
    void onOnceBeforeDraw() override {
        fCirclePath.addCircle(0, 0, 50);
        fRectPath.addRect(SkRect::MakeXYWH(-100, -50, 200, 100));
        fRRPath.addRRect(SkRRect::MakeRectXY(SkRect::MakeXYWH(-100, -50, 200, 100), 4, 4));
        fLightPos = SkPoint3::Make(-220, -330, 150);
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
                case 'T':
                    fUseAltAmbient = !fUseAltAmbient;
                    break;
                case 'S':
                    fShowSpot = !fShowSpot;
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

        // fast path
        SkRect pathRect;
        SkRRect pathRRect;
        if ((path.isOval(&pathRect) && pathRect.width() == pathRect.height()) ||
            (path.isRRect(&pathRRect) && pathRRect.allCornersCircular()) ||
            path.isRect(&pathRect)) {

            // For all of these, we outset the rect by half the radius to get our stroke shape.
            if (path.isOval(nullptr)) {
                pathRect.outset(0.5f*radius, 0.5f*radius);
                pathRRect = SkRRect::MakeOval(pathRect);
            } else if (path.isRect(nullptr)) {
                pathRect.outset(0.5f*radius, 0.5f*radius);
                pathRRect = SkRRect::MakeRectXY(pathRect, 0.5f*radius, 0.5f*radius);
            } else {
                pathRRect.outset(0.5f*radius, 0.5f*radius);
            }

            SkPaint paint;
            paint.setAntiAlias(true);
            paint.setColor(SkColorSetARGB((unsigned char)(ambientAlpha*umbraAlpha*255.999f), 
                                          0, 0, 0));
            paint.setStrokeWidth(radius);
            paint.setStyle(SkPaint::kStroke_Style);

            paint.setShader(SkGaussianEdgeShader::Make());
            canvas->drawRRect(pathRRect, paint);
        } else {
            // occlude blur
            SkRect occlRect;
            GetOcclRect(path, &occlRect);
            sk_sp<SkMaskFilter> f = SkBlurMaskFilter::Make(kNormal_SkBlurStyle,
                                                           SkBlurMask::ConvertRadiusToSigma(radius),
                                                           occlRect,
                                                           SkBlurMaskFilter::kNone_BlurFlag);

            SkPaint paint;
            paint.setAntiAlias(true);
            paint.setMaskFilter(std::move(f));
            paint.setColor(SkColorSetARGB((unsigned char)(ambientAlpha*umbraAlpha*255.999f), 
                                          0, 0, 0));
            canvas->drawPath(path, paint);

            // draw occlusion rect
#if DRAW_OCCL_RECT
            SkPaint stroke;
            stroke.setStyle(SkPaint::kStroke_Style);
            stroke.setColor(SK_ColorBLUE);
            canvas->drawRect(occlRect, stroke);
#endif
        }

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
        occlRect.fLeft /= scale;
        occlRect.fRight /= scale;
        occlRect.fTop /= scale;
        occlRect.fBottom /= scale;
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
        canvas->scale(scale, scale);
        canvas->drawPath(path, paint);

        // draw occlusion rect
#if DRAW_OCCL_RECT
        SkPaint stroke;
        stroke.setStyle(SkPaint::kStroke_Style);
        stroke.setColor(SK_ColorRED);
        canvas->drawRect(occlRect, stroke)
#endif
    }

    void drawShadowedPath(SkCanvas* canvas, const SkPath& path, SkScalar zValue, 
                          const SkPaint& paint) {
        const SkScalar kLightWidth = 3;
        const SkScalar kAmbientAlpha = 0.25f;
        const SkScalar kSpotAlpha = 0.25f;

        if (fShowAmbient) {
            if (fUseAltAmbient) {
                this->drawAmbientShadowAlt(canvas, path, zValue, kAmbientAlpha);
            } else {
                this->drawAmbientShadow(canvas, path, zValue, kAmbientAlpha);
            }
        }
        if (fShowSpot) {
            this->drawSpotShadow(canvas, path, zValue, fLightPos, kLightWidth, kSpotAlpha);
        }
        if (fShowObject) {
            canvas->drawPath(path, paint);
        }
    }

    void onDrawContent(SkCanvas* canvas) override {
        this->drawBG(canvas);

        SkPaint paint;
        paint.setAntiAlias(true);

        paint.setColor(SK_ColorWHITE);
        canvas->translate(200, 90);
        this->drawShadowedPath(canvas, fRectPath, 5, paint);

        paint.setColor(SK_ColorRED);
        canvas->translate(250, 0);
        this->drawShadowedPath(canvas, fRRPath, 5, paint);

        paint.setColor(SK_ColorBLUE);
        canvas->translate(-250, 110);
        this->drawShadowedPath(canvas, fCirclePath, 5, paint);

        paint.setColor(SK_ColorGREEN);
        canvas->translate(250, 0);
        this->drawShadowedPath(canvas, fRRPath, 5, paint);
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
