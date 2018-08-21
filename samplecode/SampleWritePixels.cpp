/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Sample.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkCornerPathEffect.h"
#include "SkGradientShader.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUTF.h"

static void create_bitmap(SkBitmap* bitmap) {
    const int W = 100;
    const int H = 100;
    bitmap->allocN32Pixels(W, H);

    SkCanvas canvas(*bitmap);
    canvas.drawColor(SK_ColorRED);
    SkPaint paint;
    paint.setColor(SK_ColorBLUE);
    canvas.drawCircle(SkIntToScalar(W)/2, SkIntToScalar(H)/2, SkIntToScalar(W)/2, paint);
}

class WritePixelsView : public Sample {
    SkPath fPath;
public:
    WritePixelsView() {}

protected:
    virtual bool onQuery(Sample::Event* evt) {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, "WritePixels");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        SkBitmap bitmap;
        create_bitmap(&bitmap);
        int x = bitmap.width() / 2;
        int y = bitmap.height() / 2;

        SkBitmap subset;
        bitmap.extractSubset(&subset, SkIRect::MakeXYWH(x, y, x, y));

        canvas->translate(SkIntToScalar(20), SkIntToScalar(20));

        canvas->writePixels(bitmap, 0, 0);
        canvas->writePixels(subset, 0, 0);
    }

private:
    typedef Sample INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new WritePixelsView(); )
