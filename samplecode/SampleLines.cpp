/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkPath.h"
#include "include/core/SkRegion.h"
#include "include/core/SkShader.h"
#include "include/core/SkStream.h"
#include "include/core/SkTime.h"
#include "include/core/SkTypeface.h"
#include "include/effects/SkCornerPathEffect.h"
#include "include/effects/SkGradientShader.h"
#include "include/utils/SkRandom.h"
#include "samplecode/DecodeFile.h"
#include "samplecode/Sample.h"
#include "src/utils/SkUTF.h"

class LinesView : public Sample {
public:
    LinesView() {}

protected:
    SkString name() override { return SkString("Lines"); }

    /*
     0x1F * x + 0x1F * (32 - x)
     */
    void drawRings(SkCanvas* canvas) {
        canvas->scale(SkIntToScalar(1)/2, SkIntToScalar(1)/2);

        SkRect  r;
        SkScalar x = SkIntToScalar(10);
        SkScalar y = SkIntToScalar(10);
        r.set(x, y, x + SkIntToScalar(100), y + SkIntToScalar(100));

        SkPaint paint;
     //   paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(SkScalarHalf(SkIntToScalar(3)));
        paint.setColor(0xFFFF8800);
     //   paint.setColor(0xFFFFFFFF);
        canvas->drawRect(r, paint);
    }

    void onDrawContent(SkCanvas* canvas) override {
        SkBitmap bm;
        decode_file("/kill.gif", &bm);
        canvas->drawBitmap(bm, 0, 0, nullptr);

        this->drawRings(canvas);
        return;

        SkPaint paint;

      //  fAlpha = 0x80;
        paint.setColor(SK_ColorWHITE);
        paint.setAlpha(fAlpha & 0xFF);
        SkRect r;

        SkScalar x = SkIntToScalar(10);
        SkScalar y = SkIntToScalar(10);
        r.set(x, y, x + SkIntToScalar(100), y + SkIntToScalar(100));
        canvas->drawRect(r, paint);
        return;

        paint.setColor(0xffffff00);            // yellow
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(SkIntToScalar(2));

//        y += SK_Scalar1/2;

        canvas->drawLine(x, y, x + SkIntToScalar(90), y + SkIntToScalar(90), paint);

        paint.setAntiAlias(true);              // with anti-aliasing
        y += SkIntToScalar(10);
        canvas->drawLine(x, y, x + SkIntToScalar(90), y + SkIntToScalar(90), paint);
    }

    Sample::Click* onFindClickHandler(SkScalar x, SkScalar y, unsigned) override {
        fAlpha = SkScalarRoundToInt(y);
        return nullptr;
    }
private:

    int fAlpha;
    typedef Sample INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new LinesView(); )
