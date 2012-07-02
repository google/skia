
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

namespace skiagm {

static void create_bitmap(SkBitmap* bitmap) {
    const int W = 100;
    const int H = 100;
    bitmap->setConfig(SkBitmap::kARGB_8888_Config, W, H);
    bitmap->allocPixels();

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
    virtual SkString onShortName() SK_OVERRIDE {
        return SkString("extractbitmap");
    }

    virtual SkISize onISize() SK_OVERRIDE {
        return make_isize(600, 600);
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        SkBitmap bitmap;
        create_bitmap(&bitmap);
        int x = bitmap.width() / 2;
        int y = bitmap.height() / 2;
        SkBitmap subset;
        bitmap.extractSubset(&subset, SkIRect::MakeXYWH(x, y, x, y));

        canvas->translate(SkIntToScalar(20), SkIntToScalar(20));

        canvas->drawBitmap(bitmap, 0, 0);
        canvas->drawBitmap(subset, 0, 0);
/*
        // Now do the same but with a device bitmap as source image
        SkRefPtr<SkDevice> primaryDevice(canvas->getDevice());
        SkRefPtr<SkDevice> secondDevice(canvas->createCompatibleDevice(
            SkBitmap::kARGB_8888_Config, bitmap.width(), 
            bitmap.height(), true));
        secondDevice->unref();
        SkCanvas secondCanvas(secondDevice.get());
        secondCanvas.writePixels(bitmap, 0, 0);

        SkBitmap deviceBitmap = secondDevice->accessBitmap(false);
        SkBitmap deviceSubset;
        deviceBitmap.extractSubset(&deviceSubset, 
             SkIRect::MakeXYWH(x, y, x, y));

        canvas->translate(SkIntToScalar(120), SkIntToScalar(0));

        canvas->drawBitmap(deviceBitmap, 0, 0);
        canvas->drawBitmap(deviceSubset, 0, 0);
*/
    }
    
private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new ExtractBitmapGM; }
static GMRegistry reg(MyFactory);

}
