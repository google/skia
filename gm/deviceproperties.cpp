/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm.h"
#include "SkTypeface.h"

namespace skiagm {

class DevicePropertiesGM : public GM {
public:
    DevicePropertiesGM() {
        this->setBGColor(0xFFFFFFFF);
    }

    virtual ~DevicePropertiesGM() {
    }

protected:
    virtual SkString onShortName() {
        return SkString("deviceproperties");
    }

    virtual SkISize onISize() {
        return make_isize(1450, 750);
    }

    static void rotate_about(SkCanvas* canvas,
                             SkScalar degrees,
                             SkScalar px, SkScalar py) {
        canvas->translate(px, py);
        canvas->rotate(degrees);
        canvas->translate(-px, -py);
    }

    virtual void onDraw(SkCanvas* originalCanvas) {
        SkISize size = this->getISize();
        SkBitmap bitmap;
        bitmap.setConfig(SkBitmap::kARGB_8888_Config, size.width(), size.height());
        bitmap.allocPixels();
        SkDeviceProperties properties = SkDeviceProperties::Make(
            SkDeviceProperties::Geometry::Make(SkDeviceProperties::Geometry::kVertical_Orientation,
                                               SkDeviceProperties::Geometry::kBGR_Layout),
            SK_Scalar1);
        SkBitmapDevice device(bitmap, properties);
        SkCanvas canvas(&device);
        canvas.drawColor(SK_ColorWHITE);

        SkPaint paint;

        paint.setAntiAlias(true);
        paint.setLCDRenderText(true);
        //With freetype the default (normal hinting) can be really ugly.
        //Most distros now set slight (vertical hinting only) in any event.
        paint.setHinting(SkPaint::kSlight_Hinting);
        SkSafeUnref(paint.setTypeface(SkTypeface::CreateFromName("Times Roman", SkTypeface::kNormal)));

        const char* text = "Hamburgefons ooo mmm";
        const size_t textLen = strlen(text);

        for (int j = 0; j < 2; ++j) {
            for (int i = 0; i < 6; ++i) {
                SkScalar x = SkIntToScalar(10);
                SkScalar y = SkIntToScalar(20);

                SkAutoCanvasRestore acr(&canvas, true);
                canvas.translate(SkIntToScalar(50 + i * 230),
                                  SkIntToScalar(20));
                rotate_about(&canvas, SkIntToScalar(i * 5), x, y * 10);

                {
                    SkPaint p;
                    p.setAntiAlias(true);
                    SkRect r;
                    r.set(x - SkIntToScalar(3), SkIntToScalar(15),
                          x - SkIntToScalar(1), SkIntToScalar(280));
                    canvas.drawRect(r, p);
                }

                int index = 0;
                for (int ps = 6; ps <= 22; ps++) {
                    paint.setTextSize(SkIntToScalar(ps));
                    canvas.drawText(text, textLen, x, y, paint);
                    y += paint.getFontMetrics(NULL);
                    index += 1;
                }
            }
            canvas.translate(0, SkIntToScalar(360));
            paint.setSubpixelText(true);
        }
        originalCanvas->drawBitmap(bitmap, 0, 0);
    }

#ifdef SK_BUILD_FOR_ANDROID
    virtual uint32_t onGetFlags() const SK_OVERRIDE {
        // On android, we fail due to bad gpu drivers (it seems) by adding too
        // much to our text atlas (texture).
        return kSkipGPU_Flag;
    }
#endif

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new DevicePropertiesGM; }
static GMRegistry reg(MyFactory);

}
