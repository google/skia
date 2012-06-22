/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkLightingImageFilter.h"

#define WIDTH 330
#define HEIGHT 220

namespace skiagm {

class ImageLightingGM : public GM {
public:
    ImageLightingGM() : fInitialized(false) {
        this->setBGColor(0xFF000000);
    }
    
protected:
    virtual SkString onShortName() {
        return SkString("lighting");
    }

    void make_bitmap() {
        fBitmap.setConfig(SkBitmap::kARGB_8888_Config, 100, 100);
        fBitmap.allocPixels();
        SkDevice device(fBitmap);
        SkCanvas canvas(&device);
        canvas.clear(0x00000000);
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(0xFFFFFFFF);
        paint.setTextSize(SkIntToScalar(96));
        const char* str = "e";
        canvas.drawText(str, strlen(str), SkIntToScalar(20), SkIntToScalar(70), paint);
    }

    virtual SkISize onISize() {
        return make_isize(WIDTH, HEIGHT);
    }

    virtual void onDraw(SkCanvas* canvas) {
        if (!fInitialized) {
            make_bitmap();
            fInitialized = true;
        }
        SkPoint3 pointLocation(0, 0, SkIntToScalar(10));
        SkScalar azimuthRad = SkDegreesToRadians(SkIntToScalar(225));
        SkScalar elevationRad = SkDegreesToRadians(SkIntToScalar(5));
        SkPoint3 distantDirection(SkScalarMul(SkScalarCos(azimuthRad), SkScalarCos(elevationRad)),
                                  SkScalarMul(SkScalarSin(azimuthRad), SkScalarCos(elevationRad)),
                                  SkScalarSin(elevationRad));
        SkPoint3 spotLocation(SkIntToScalar(-10), SkIntToScalar(-10), SkIntToScalar(20));
        SkPoint3 spotTarget(SkIntToScalar(40), SkIntToScalar(40), 0);
        SkScalar spotExponent = SK_Scalar1;
        SkScalar cutoffAngle = SkIntToScalar(15);
        SkScalar kd = SkIntToScalar(2);
        SkScalar ks = SkIntToScalar(1);
        SkScalar shininess = SkIntToScalar(8);
        SkScalar surfaceScale = SkIntToScalar(1);
        SkColor white(0xFFFFFFFF);
        SkPaint paint;
        paint.setImageFilter(SkLightingImageFilter::CreatePointLitDiffuse(pointLocation, white, surfaceScale, kd))->unref();
        canvas->drawSprite(fBitmap, 0, 0, &paint);
        paint.setImageFilter(SkLightingImageFilter::CreateDistantLitDiffuse(distantDirection, white, surfaceScale, kd))->unref();
        canvas->drawSprite(fBitmap, 110, 0, &paint);
        paint.setImageFilter(SkLightingImageFilter::CreateSpotLitDiffuse(spotLocation, spotTarget, spotExponent, cutoffAngle, white, surfaceScale, kd))->unref();
        canvas->drawSprite(fBitmap, 220, 0, &paint);
        paint.setImageFilter(SkLightingImageFilter::CreatePointLitSpecular(pointLocation, white, surfaceScale, ks, shininess))->unref();
        canvas->drawSprite(fBitmap, 0, 110, &paint);
        paint.setImageFilter(SkLightingImageFilter::CreateDistantLitSpecular(distantDirection, white, surfaceScale, ks, shininess))->unref();
        canvas->drawSprite(fBitmap, 110, 110, &paint);
        paint.setImageFilter(SkLightingImageFilter::CreateSpotLitSpecular(spotLocation, spotTarget, spotExponent, cutoffAngle, white, surfaceScale, ks, shininess))->unref();
        canvas->drawSprite(fBitmap, 220, 110, &paint);
    }
    
private:
    typedef GM INHERITED;
    SkBitmap fBitmap;
    bool fInitialized;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new ImageLightingGM; }
static GMRegistry reg(MyFactory);

}
