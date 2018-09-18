/*
* Copyright 2017 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "gm.h"
#include "SkCanvas.h"
#include "SkPath.h"
#include "SkShadowUtils.h"

enum BackgroundMode {
    kLightBackground,
    kDarkBackground
};

static constexpr int kW = 1225;
static constexpr int kH = 550;

void draw_content(SkCanvas* canvas, BackgroundMode mode) {
    const SkScalar kLightWidth = 600;
    const SkScalar kAmbientAlpha = 0.03f;
    const SkScalar kSpotAlpha = 0.25f;

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
    if (mode == kDarkBackground) {
        canvas->drawColor(0xFF111111);
    } else {
        canvas->drawColor(0xFFEAEAEA);
    }

    SkPath path;
    path.addRect(SkRect::MakeXYWH(-50, -50, 100, 100));

    SkPoint3 lightPos = { 75, -400, 600 };
    SkPoint3 zPlaneParams = SkPoint3::Make(0, 0, 16);
    SkScalar yPos = 75;

    for (int row = 0; row < 3; ++row) {
        lightPos.fX = 75;
        SkScalar xPos = 75;
        for (int col = 0; col < 10; ++col) {
            SkColor color = kColors[10 * row + col];
            paint.setColor(color);

            canvas->save();
            canvas->translate(xPos, yPos);

            SkColor baseAmbient = SkColorSetARGB(kAmbientAlpha*SkColorGetA(color),
                                                 SkColorGetR(color), SkColorGetG(color),
                                                 SkColorGetB(color));
            SkColor baseSpot = SkColorSetARGB(kSpotAlpha*SkColorGetA(color),
                                              SkColorGetR(color), SkColorGetG(color),
                                              SkColorGetB(color));
            SkColor tonalAmbient, tonalSpot;
            SkShadowUtils::ComputeTonalColors(baseAmbient, baseSpot, &tonalAmbient, &tonalSpot);
            SkShadowUtils::DrawShadow(canvas, path, zPlaneParams,
                                      lightPos, kLightWidth, tonalAmbient, tonalSpot);
            canvas->drawPath(path, paint);
            canvas->restore();

            lightPos.fX += 120;
            xPos += 120;
        }

        lightPos.fY += 200;
        yPos += 200;
    }
}

DEF_SIMPLE_GM(tonalshadows_light, canvas, kW, kH) {
    draw_content(canvas, kLightBackground);
}

DEF_SIMPLE_GM(tonalshadows_dark, canvas, kW, kH) {
    draw_content(canvas, kDarkBackground);
}



