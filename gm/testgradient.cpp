/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"
#include "include/effects/SkGradientShader.h"

class TestGradientGM : public skiagm::GM {
public:
    TestGradientGM() {}

protected:
    SkString getName() const override { return SkString("testgradient"); }

    SkISize getISize() override { return SkISize::Make(800, 800); }

    void onDraw(SkCanvas* canvas) override {
        // Set up a gradient paint for a rect.
        // And non-gradient paint for other objects.
        canvas->drawColor(SK_ColorWHITE);

        SkPaint paint;
        paint.setStyle(SkPaint::kFill_Style);
        paint.setAntiAlias(true);
        paint.setStrokeWidth(4);
        paint.setColor(0xFFFE938C);

        SkRect rect = SkRect::MakeXYWH(10, 10, 100, 160);

        SkPoint points[2] = {
            SkPoint::Make(0.0f, 0.0f),
            SkPoint::Make(256.0f, 256.0f)
        };
        SkColor colors[2] = {SK_ColorBLUE, SK_ColorYELLOW};
        SkPaint newPaint(paint);
        newPaint.setShader(SkGradientShader::MakeLinear(
                points, colors, nullptr, 2, SkTileMode::kClamp));
        canvas->drawRect(rect, newPaint);

        SkRRect oval;
        oval.setOval(rect);
        oval.offset(40, 80);
        paint.setColor(0xFFE6B89C);
        canvas->drawRRect(oval, paint);

        paint.setColor(0xFF9CAFB7);
        canvas->drawCircle(180, 50, 25, paint);

        rect.offset(80, 50);
        paint.setColor(0xFF4281A4);
        paint.setStyle(SkPaint::kStroke_Style);
        canvas->drawRoundRect(rect, 10, 10, paint);
    }

private:
    using INHERITED = skiagm::GM;
};

// Register the GM
DEF_GM( return new TestGradientGM; )
