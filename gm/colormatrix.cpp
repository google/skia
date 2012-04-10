/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkColorMatrixFilter.h"

#define WIDTH 500
#define HEIGHT 500

class SkOnce {
public:
    SkOnce() : fOnce(false) {};

    bool once() const {
        if (fOnce) {
            return false;
        }
        fOnce = true;
        return true;
    }
    
private:
    mutable bool fOnce;
};

namespace skiagm {

class ColorMatrixGM : public GM {
    SkOnce fOnce;
    void init() {
        if (fOnce.once()) {
            fBitmap = createBitmap(64, 64);
        }
    }

public:
    ColorMatrixGM() {
        this->setBGColor(0xFF808080);
    }
    
protected:
    virtual SkString onShortName() {
        return SkString("colormatrix");
    }

    virtual SkISize onISize() {
        return make_isize(WIDTH, HEIGHT);
    }

    SkBitmap createBitmap(int width, int height) {
        SkBitmap bm;
        bm.setConfig(SkBitmap::kARGB_8888_Config, width, height);
        bm.allocPixels();
        SkCanvas canvas(bm);
        canvas.clear(0x0);
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                SkPaint paint;
                paint.setColor(SkColorSetARGB(255, x * 255 / width, y * 255 / height, 0));
                canvas.drawRect(SkRect::MakeXYWH(SkIntToScalar(x),
                    SkIntToScalar(y), SK_Scalar1, SK_Scalar1), paint);
            }
        }
        return bm;
    }
    virtual void onDraw(SkCanvas* canvas) {
        this->init();

        SkPaint paint;
        SkColorMatrix matrix;
        SkColorMatrixFilter* filter = new SkColorMatrixFilter();
        paint.setColorFilter(filter)->unref();

        matrix.setIdentity();
        filter->setMatrix(matrix);
        canvas->drawBitmap(fBitmap, 0, 0, &paint);

        matrix.setRotate(SkColorMatrix::kR_Axis, 90);
        filter->setMatrix(matrix);
        canvas->drawBitmap(fBitmap, 80, 0, &paint);

        matrix.setRotate(SkColorMatrix::kG_Axis, 90);
        filter->setMatrix(matrix);
        canvas->drawBitmap(fBitmap, 160, 0, &paint);

        matrix.setRotate(SkColorMatrix::kB_Axis, 90);
        filter->setMatrix(matrix);
        canvas->drawBitmap(fBitmap, 240, 0, &paint);

        matrix.setSaturation(SkFloatToScalar(0.0f));
        filter->setMatrix(matrix);
        canvas->drawBitmap(fBitmap, 0, 80, &paint);

        matrix.setSaturation(SkFloatToScalar(0.5f));
        filter->setMatrix(matrix);
        canvas->drawBitmap(fBitmap, 80, 80, &paint);

        matrix.setSaturation(SkFloatToScalar(1.0f));
        filter->setMatrix(matrix);
        canvas->drawBitmap(fBitmap, 160, 80, &paint);

        matrix.setSaturation(SkFloatToScalar(2.0f));
        filter->setMatrix(matrix);
        canvas->drawBitmap(fBitmap, 240, 80, &paint);

        matrix.setRGB2YUV();
        filter->setMatrix(matrix);
        canvas->drawBitmap(fBitmap, 0, 160, &paint);

        matrix.setYUV2RGB();
        filter->setMatrix(matrix);
        canvas->drawBitmap(fBitmap, 80, 160, &paint);

        SkScalar s1 = SK_Scalar1;
        SkScalar s255 = SkIntToScalar(255);
        // Move red into alpha, set color to white
        SkScalar data[20] = {
            0,  0, 0, 0, s255,
            0,  0, 0, 0, s255,
            0,  0, 0, 0, s255,
            s1, 0, 0, 0, 0,
        };

        filter->setArray(data);
        canvas->drawBitmap(fBitmap, 160, 160, &paint);
    }
    
private:
    SkBitmap fBitmap;
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new ColorMatrixGM; }
static GMRegistry reg(MyFactory);

}
