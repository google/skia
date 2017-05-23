
/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SampleCode.h"
#include "Resources.h"
#include "SkCanvas.h"
#include "SkImage.h"
#include "SkPath.h"
#include "SkPoint3.h"
#include "SkShadowUtils.h"

////////////////////////////////////////////////////////////////////////////
// Sample to compare the Material Design shadow reference to our results

class ShadowRefView : public SampleView {
    SkPath         fSquarePath;
    sk_sp<SkImage> fReferenceImage;

    bool      fShowAmbient;
    bool      fShowSpot;
    bool      fUseAlt;
    bool      fShowObject;

public:
    ShadowRefView()
        : fShowAmbient(true)
        , fShowSpot(true)
        , fUseAlt(false)
        , fShowObject(true) {}

protected:
    void onOnceBeforeDraw() override {
        fSquarePath.addRRect(SkRRect::MakeRectXY(SkRect::MakeXYWH(-130, -128.5, 130, 128.5), 4, 4));
        fReferenceImage = GetResourceAsImage("shadowreference.png");
    }

    // overrides from SkEventSink
    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "ShadowReference");
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
        canvas->drawColor(0xFFFFFFFF);
        canvas->drawImage(fReferenceImage, 10, 30);
    }

    void drawShadowedPath(SkCanvas* canvas, const SkPath& path,
                          const SkPoint3& zPlaneParams,
                          const SkPaint& paint, SkScalar ambientAlpha,
                          const SkPoint3& lightPos, SkScalar lightWidth, SkScalar spotAlpha) {
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
        const SkScalar kLightWidth = 1600;
        const SkScalar kAmbientAlpha = 0.03f;
        const SkScalar kSpotAlpha = 0.35f;
        const SkScalar kDP = 4;

        SkPaint paint;
        paint.setAntiAlias(true);

        SkPoint3 lightPos = { 175, -800, kDP*600 };
        SkPoint3 zPlaneParams = SkPoint3::Make(0, 0, 0);

        paint.setColor(SK_ColorWHITE);

        canvas->save();
        canvas->clipRect(SkRect::MakeXYWH(45, 75, 122, 250));
        canvas->translate(230, 254.25);
        zPlaneParams.fZ = kDP*2;
        this->drawShadowedPath(canvas, fSquarePath, zPlaneParams, paint, kAmbientAlpha,
                               lightPos, kLightWidth, kSpotAlpha);
        canvas->restore();

        lightPos.fX += 320;
        canvas->save();
        canvas->clipRect(SkRect::MakeXYWH(365, 75, 122, 250));
        canvas->translate(550, 254.25);
        zPlaneParams.fZ = kDP * 3;
        this->drawShadowedPath(canvas, fSquarePath, zPlaneParams, paint, kAmbientAlpha,
                               lightPos, kLightWidth, kSpotAlpha);
        canvas->restore();

        lightPos.fX += 320;
        canvas->save();
        canvas->clipRect(SkRect::MakeXYWH(685, 75, 122, 250));
        canvas->translate(870, 254.25);
        zPlaneParams.fZ = kDP * 4;
        this->drawShadowedPath(canvas, fSquarePath, zPlaneParams, paint, kAmbientAlpha,
                               lightPos, kLightWidth, kSpotAlpha);
        canvas->restore();

        lightPos.fX += 320;
        canvas->save();
        canvas->clipRect(SkRect::MakeXYWH(1005, 75, 122, 250));
        canvas->translate(1190, 254.25);
        zPlaneParams.fZ = kDP * 6;
        this->drawShadowedPath(canvas, fSquarePath, zPlaneParams, paint, kAmbientAlpha,
                               lightPos, kLightWidth, kSpotAlpha);
        canvas->restore();

        lightPos.fX += 320;
        canvas->save();
        canvas->clipRect(SkRect::MakeXYWH(1325, 75, 122, 250));
        canvas->translate(1510, 254.25);
        zPlaneParams.fZ = kDP * 8;
        this->drawShadowedPath(canvas, fSquarePath, zPlaneParams, paint, kAmbientAlpha,
                               lightPos, kLightWidth, kSpotAlpha);
        canvas->restore();

        lightPos.fX += 320;
        canvas->save();
        canvas->clipRect(SkRect::MakeXYWH(1645, 75, 122, 250));
        canvas->translate(1830, 254.25);
        zPlaneParams.fZ = kDP * 16;
        this->drawShadowedPath(canvas, fSquarePath, zPlaneParams, paint, kAmbientAlpha,
                               lightPos, kLightWidth, kSpotAlpha);
        canvas->restore();

    }

private:
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new ShadowRefView; }
static SkViewRegister reg(MyFactory);
