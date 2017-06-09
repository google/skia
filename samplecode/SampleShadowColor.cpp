
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
// Sample to demonstrate tonal color shadows

class ShadowColorView : public SampleView {
    SkPath    fRectPath;
    int       fZIndex;

    bool      fShowAmbient;
    bool      fShowSpot;
    bool      fUseAlt;
    bool      fShowObject;
    bool      fTwoPassColor;
    bool      fDarkBackground;

public:
    ShadowColorView()
        : fZIndex(8)
        , fShowAmbient(true)
        , fShowSpot(true)
        , fUseAlt(false)
        , fShowObject(true)
        , fTwoPassColor(false)
        , fDarkBackground(false) {}

protected:
    void onOnceBeforeDraw() override {
        fRectPath.addRect(SkRect::MakeXYWH(-50, -50, 100, 100));
    }

    // overrides from SkEventSink
    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "ShadowColor");
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
                case 'X':
                    fTwoPassColor = !fTwoPassColor;
                    handled = true;
                    break;
                case 'Z':
                    fDarkBackground = !fDarkBackground;
                    handled = true;
                    break;
                case '>':
                    fZIndex = SkTMin(9, fZIndex+1);
                    handled = true;
                    break;
                case '<':
                    fZIndex = SkTMax(0, fZIndex-1);
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

        if (fTwoPassColor) {
            SkShadowUtils::DrawShadow(canvas, path, zPlaneParams,
                                      lightPos, lightWidth,
                                      ambientAlpha, 0, SK_ColorBLACK, flags);

            if (paint.getColor() != SK_ColorBLACK) {
                SkColor color = paint.getColor();

                uint8_t max = SkTMax(SkTMax(SkColorGetR(color), SkColorGetG(color)),
                                     SkColorGetB(color));
                uint8_t min = SkTMin(SkTMin(SkColorGetR(color), SkColorGetG(color)),
                                     SkColorGetB(color));
                SkScalar luminance = 0.5f*(max + min) / 255.f;
                SkScalar alpha = (.6 - .4*luminance)*luminance*luminance + 0.3f;
                spotAlpha -= (alpha - 0.3f)*.5f;

                SkShadowUtils::DrawShadow(canvas, path, zPlaneParams,
                                          lightPos, lightWidth,
                                          0, alpha, paint.getColor(), flags);
            }

            SkShadowUtils::DrawShadow(canvas, path, zPlaneParams,
                                      lightPos, lightWidth,
                                      0, spotAlpha, SK_ColorBLACK, flags);
        } else {
            flags |= SkShadowFlags::kTonalColor_ShadowFlag;
            SkShadowUtils::DrawShadow(canvas, path, zPlaneParams,
                                      lightPos, lightWidth,
                                      ambientAlpha, spotAlpha, paint.getColor(), flags);
        }
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
        const SkScalar kLightWidth = 600;
        const SkScalar kAmbientAlpha = 0.03f;
        const SkScalar kSpotAlpha = 0.25f;

        const SkScalar kZValues[10] = { 1, 2, 3, 4, 6, 8, 9, 12, 16, 24 };

        const SkColor kColors[30] = {
            // purples
            0xFF3A0072, 0xFF5D0099, 0xFF7F12B2, 0xFFA02AD1, 0xFFC245E5,
            0xFFE95AF9, 0xFFFC79F0, 0xFFFDA6F0, 0xFFFFCCF8, 0xFFFFE1F9,
            // oranges
            0xFFEA3200, 0xFFFF4E00, 0xFFFF7300, 0xFFFF9100, 0xFFFFB000,
            0xFFFFCE00, 0xFFFFE000, 0xFFFFF64D, 0xFFFFF98F, 0xFFFFFBCC,
            // teals
            0xFF004D51, 0xFF066266, 0xFF057F7F, 0xFF009999, 0xFF00B2B2,
            0xFF15CCBE, 0xFF25E5CE, 0xFF2CFFE0, 0xFF80FFEA, 0xFFB3FFF0
        };

        SkPaint paint;
        paint.setAntiAlias(true);
        if (fDarkBackground) {
            canvas->drawColor(0xFF111111);
            paint.setColor(SK_ColorWHITE);
        } else {
            canvas->drawColor(0xFFEAEAEA);
            paint.setColor(SK_ColorBLACK);
        }

        if (fTwoPassColor) {
            canvas->drawText("Two pass", 8, 10, 15, paint);
        } else {
            canvas->drawText("One pass", 8, 10, 15, paint);
        }

        SkPoint3 lightPos = { 75, -400, 600 };
        SkPoint3 zPlaneParams = SkPoint3::Make(0, 0, kZValues[fZIndex]);
        SkScalar yPos = 75;

        for (int row = 0; row < 3; ++row) {
            lightPos.fX = 75;
            SkScalar xPos = 75;
            for (int col = 0; col < 10; ++col) {
                paint.setColor(kColors[10*row + col]);

                canvas->save();
                canvas->translate(xPos, yPos);
                this->drawShadowedPath(canvas, fRectPath, zPlaneParams, paint, kAmbientAlpha,
                                       lightPos, kLightWidth, kSpotAlpha);
                canvas->restore();

                lightPos.fX += 120;
                xPos += 120;
            }

            lightPos.fY += 200;
            yPos += 200;
        }
    }

private:
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new ShadowColorView; }
static SkViewRegister reg(MyFactory);
