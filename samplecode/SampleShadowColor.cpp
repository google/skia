
/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "samplecode/Sample.h"
#include "tools/Resources.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkImage.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint3.h"
#include "include/utils/SkShadowUtils.h"

////////////////////////////////////////////////////////////////////////////
// Sample to demonstrate tonal color shadows

class ShadowColorView : public Sample {
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

    SkString name() override { return SkString("ShadowColor"); }

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
                case 'X':
                    fTwoPassColor = !fTwoPassColor;
                    handled = true;
                    break;
                case 'Z':
                    fDarkBackground = !fDarkBackground;
                    handled = true;
                    break;
                case '>':
                    fZIndex = std::min(9, fZIndex+1);
                    handled = true;
                    break;
                case '<':
                    fZIndex = std::max(0, fZIndex-1);
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
            SkColor ambientColor = SkColorSetARGB(ambientAlpha*255, 0, 0, 0);
            SkShadowUtils::DrawShadow(canvas, path, zPlaneParams,
                                      lightPos, lightWidth,
                                      ambientColor, SK_ColorTRANSPARENT, flags);

            if (paint.getColor() != SK_ColorBLACK) {
                SkColor color = paint.getColor();

                uint8_t max = std::max(std::max(SkColorGetR(color), SkColorGetG(color)),
                                     SkColorGetB(color));
                uint8_t min = std::min(std::min(SkColorGetR(color), SkColorGetG(color)),
                                     SkColorGetB(color));
                SkScalar luminance = 0.5f*(max + min) / 255.f;
                SkScalar alpha = (.6 - .4*luminance)*luminance*luminance + 0.3f;
                spotAlpha -= (alpha - 0.3f)*.5f;
                SkColor spotColor = SkColorSetARGB(alpha*SkColorGetA(color), SkColorGetR(color),
                                                   SkColorGetG(color), SkColorGetB(color));

                SkShadowUtils::DrawShadow(canvas, path, zPlaneParams,
                                          lightPos, lightWidth,
                                          SK_ColorTRANSPARENT, spotColor, flags);
            }

            SkColor spotGreyscale = SkColorSetARGB(spotAlpha * 255, 0, 0, 0);
            SkShadowUtils::DrawShadow(canvas, path, zPlaneParams,
                                      lightPos, lightWidth,
                                      SK_ColorTRANSPARENT, spotGreyscale, flags);
        } else {
            SkColor color = paint.getColor();
            SkColor baseAmbient = SkColorSetARGB(ambientAlpha*SkColorGetA(color),
                                                 SkColorGetR(color), SkColorGetG(color),
                                                 SkColorGetB(color));
            SkColor baseSpot = SkColorSetARGB(spotAlpha*SkColorGetA(color),
                                              SkColorGetR(color), SkColorGetG(color),
                                              SkColorGetB(color));
            SkColor tonalAmbient, tonalSpot;
            SkShadowUtils::ComputeTonalColors(baseAmbient, baseSpot, &tonalAmbient, &tonalSpot);
            SkShadowUtils::DrawShadow(canvas, path, zPlaneParams,
                                      lightPos, lightWidth,
                                      tonalAmbient, tonalSpot, flags);
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

        SkFont font;
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
            canvas->drawString("Two pass", 10, 15, font, paint);
        } else {
            canvas->drawString("One pass", 10, 15, font, paint);
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
    typedef Sample INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new ShadowColorView(); )
