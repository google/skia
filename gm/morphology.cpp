/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkMorphologyImageFilter.h"

#define WIDTH 700
#define HEIGHT 560

namespace skiagm {

class MorphologyGM : public GM {
public:
    MorphologyGM() {
        this->setBGColor(0xFF000000);
        fOnce = false;
    }

protected:
    virtual SkString onShortName() {
        return SkString("morphology");
    }

    void make_bitmap() {
        fBitmap.setConfig(SkBitmap::kARGB_8888_Config, 135, 135);
        fBitmap.allocPixels();
        SkBitmapDevice device(fBitmap);
        SkCanvas canvas(&device);
        canvas.clear(0x0);
        SkPaint paint;
        paint.setAntiAlias(true);
        const char* str1 = "ABC";
        const char* str2 = "XYZ";
        paint.setColor(0xFFFFFFFF);
        paint.setTextSize(64);
        canvas.drawText(str1, strlen(str1), 10, 55, paint);
        canvas.drawText(str2, strlen(str2), 10, 110, paint);
    }

    virtual SkISize onISize() {
        return make_isize(WIDTH, HEIGHT);
    }

    void drawClippedBitmap(SkCanvas* canvas, const SkPaint& paint, int x, int y) {
        canvas->save();
        canvas->translate(SkIntToScalar(x), SkIntToScalar(y));
        canvas->clipRect(SkRect::MakeWH(
          SkIntToScalar(fBitmap.width()), SkIntToScalar(fBitmap.height())));
        canvas->drawBitmap(fBitmap, 0, 0, &paint);
        canvas->restore();
    }

    virtual void onDraw(SkCanvas* canvas) {
        if (!fOnce) {
            make_bitmap();
            fOnce = true;
        }
        struct {
            int fWidth, fHeight;
            int fRadiusX, fRadiusY;
        } samples[] = {
            { 140, 140,   0,   0 },
            { 140, 140,   0,   2 },
            { 140, 140,   2,   0 },
            { 140, 140,   2,   2 },
            {  24,  24,  25,  25 },
        };
        SkPaint paint;
        SkImageFilter::CropRect cropRect(SkRect::MakeXYWH(25, 20, 100, 80));

        for (unsigned j = 0; j < 4; ++j) {
            for (unsigned i = 0; i < SK_ARRAY_COUNT(samples); ++i) {
                const SkImageFilter::CropRect* cr = j & 0x02 ? &cropRect : NULL;
                if (j & 0x01) {
                    paint.setImageFilter(new SkErodeImageFilter(
                        samples[i].fRadiusX,
                        samples[i].fRadiusY,
                        NULL,
                        cr))->unref();
                } else {
                    paint.setImageFilter(new SkDilateImageFilter(
                        samples[i].fRadiusX,
                        samples[i].fRadiusY,
                        NULL,
                        cr))->unref();
                }
                drawClippedBitmap(canvas, paint, i * 140, j * 140);
            }
        }
    }

private:
    typedef GM INHERITED;
    SkBitmap fBitmap;
    bool fOnce;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new MorphologyGM; }
static GMRegistry reg(MyFactory);

}
