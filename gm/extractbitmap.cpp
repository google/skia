/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkString.h"
#include "SkSurface.h"

namespace skiagm {

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

class ExtractBitmapGM : public GM {
public:
    ExtractBitmapGM() {}

protected:
    // overrides from SkEventSink
    SkString onShortName() override {
        return SkString("extractbitmap");
    }

    SkISize onISize() override {
        return SkISize::Make(600, 600);
    }

    void onDraw(SkCanvas* canvas) override {
        SkBitmap bitmap;
        create_bitmap(&bitmap);
        int x = bitmap.width() / 2;
        int y = bitmap.height() / 2;

        canvas->translate(SkIntToScalar(20), SkIntToScalar(20));

        canvas->drawBitmap(bitmap, 0, 0);

        {
            // Do some subset drawing. This will test that an SkGPipe properly
            // handles the case where bitmaps share a pixelref
            // Draw the bottom right fourth of the bitmap over the top left
            SkBitmap subset;
            bitmap.extractSubset(&subset, SkIRect::MakeXYWH(x, y, x, y));
            canvas->drawBitmap(subset, 0, 0);
            // Draw the top left corner over the bottom right
            bitmap.extractSubset(&subset, SkIRect::MakeWH(x, y));
            canvas->drawBitmap(subset, SkIntToScalar(x), SkIntToScalar(y));
            // Draw a subset which has the same height and pixelref offset but a
            // different width
            bitmap.extractSubset(&subset, SkIRect::MakeWH(x, bitmap.height()));
            SkAutoCanvasRestore autoRestore(canvas, true);
            canvas->translate(0, SkIntToScalar(bitmap.height() + 20));
            canvas->drawBitmap(subset, 0, 0);
            // Now draw a subet which has the same width and pixelref offset but
            // a different height
            bitmap.extractSubset(&subset, SkIRect::MakeWH(bitmap.height(), y));
            canvas->translate(0, SkIntToScalar(bitmap.height() + 20));
            canvas->drawBitmap(subset, 0, 0);
        }
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new ExtractBitmapGM; }
static GMRegistry reg(MyFactory);

}
