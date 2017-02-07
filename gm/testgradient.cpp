/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

// Include anything else that you need here
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkRRect.h"

class NewGM : public skiagm::GM {
public:
    NewGM() {}

protected:
    SkString onShortName() override {
        return SkString("testgradient");
    }


    // Set the size of the gm
    SkISize onISize() override {
        return SkISize::Make(800, 800);
    }

    void onDraw(SkCanvas* canvas) override {
        // Draw anything to the skia canvas
        canvas->drawColor(SK_ColorWHITE);

        SkPaint paint;
        paint.setStyle(SkPaint::kFill_Style);
        paint.setAntiAlias(true);
        paint.setStrokeWidth(4);
        paint.setColor(0xffFE938C);
        
        SkRect rect = SkRect::MakeXYWH(10, 10, 100, 160);
        
        SkPoint points[2] = {
            SkPoint::Make(0.0f, 0.0f),
            SkPoint::Make(256.0f, 256.0f)
        };
        SkColor colors[2] = {SK_ColorBLUE, SK_ColorYELLOW};
        SkPaint newPaint(paint);
        newPaint.setShader(SkGradientShader::MakeLinear(
                points, colors, nullptr, 2,
                SkShader::kClamp_TileMode, 0, nullptr));
        canvas->drawRect(rect, newPaint);
        
        SkRRect oval;
        oval.setOval(rect);
        oval.offset(40, 80);
        paint.setColor(0xffE6B89C);
        canvas->drawRRect(oval, paint);
        
        paint.setColor(0xff9CAFB7);
        canvas->drawCircle(180, 50, 25, paint);
        
        rect.offset(80, 50);
        paint.setColor(0xff4281A4);
        paint.setStyle(SkPaint::kStroke_Style);
        canvas->drawRoundRect(rect, 10, 10, paint);
    }

private:
    typedef skiagm::GM INHERITED;
};

// Register the GM
DEF_GM( return new NewGM; )
