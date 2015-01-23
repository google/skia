/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "../src/fonts/SkGScalerContext.h"

class ColorTypeGM : public skiagm::GM {
public:
    ColorTypeGM() {
        const SkColor colors[] = {
            SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE,
            SK_ColorMAGENTA, SK_ColorCYAN, SK_ColorYELLOW
        };
        SkMatrix local;
        local.setRotate(180);
        SkShader* s = SkGradientShader::CreateSweep(0,0, colors, NULL,
                                                    SK_ARRAY_COUNT(colors), 0, &local);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setShader(s)->unref();

        SkTypeface* orig = sk_tool_utils::create_portable_typeface("Times",
                                                            SkTypeface::kBold);
        if (NULL == orig) {
            orig = SkTypeface::RefDefault();
        }
        fColorType = SkNEW_ARGS(SkGTypeface, (orig, paint));
        orig->unref();
    }

    virtual ~ColorTypeGM() {
        fColorType->unref();
    }

protected:
    SkString onShortName() SK_OVERRIDE {
        return SkString("colortype");
    }

    SkISize onISize() SK_OVERRIDE {
        return SkISize::Make(640, 480);
    }

    void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setTypeface(fColorType);

        for (SkScalar size = 10; size <= 100; size += 10) {
            paint.setTextSize(size);
            canvas->translate(0, paint.getFontMetrics(NULL));
            canvas->drawText("Hamburgefons", 12, 10, 10, paint);
        }
    }

private:
    SkTypeface* fColorType;

    typedef skiagm::GM INHERITED;
};

DEF_GM( return SkNEW(ColorTypeGM); )
