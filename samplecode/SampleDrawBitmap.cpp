
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkDevice.h"

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

class DrawBitmapView : public SampleView {
    SkPath fPath;
public:
	DrawBitmapView() {}
    
protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "DrawBitmap");
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

        canvas->drawBitmap(bitmap, 0, 0);
        canvas->drawBitmap(subset, 0, 0);

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

    }
    
private:
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new DrawBitmapView; }
static SkViewRegister reg(MyFactory);
