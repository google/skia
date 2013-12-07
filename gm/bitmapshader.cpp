
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm.h"

#include "SkBitmap.h"
#include "SkPaint.h"
#include "SkShader.h"

namespace skiagm {

static void draw_bm(SkBitmap* bm) {
    SkPaint bluePaint;
    bluePaint.setColor(SK_ColorBLUE);

    bm->setConfig(SkBitmap::kARGB_8888_Config, 20, 20);
    bm->allocPixels();
    bm->eraseColor(SK_ColorRED);

    SkCanvas canvas(*bm);
    canvas.drawCircle(10, 10, 5, bluePaint);
}

static void draw_mask(SkBitmap* bm) {
    SkPaint circlePaint;
    circlePaint.setColor(SK_ColorBLACK);

    bm->setConfig(SkBitmap::kA8_Config, 20, 20);
    bm->allocPixels();
    bm->eraseColor(SK_ColorTRANSPARENT);

    SkCanvas canvas(*bm);
    canvas.drawCircle(10, 10, 10, circlePaint);
}

class BitmapShaderGM : public GM {
public:

    BitmapShaderGM() {
        this->setBGColor(SK_ColorGRAY);
        draw_bm(&fBitmap);
        draw_mask(&fMask);
    }

protected:
    virtual SkString onShortName() {
        return SkString("bitmapshaders");
    }

    virtual SkISize onISize() {
        return make_isize(75, 100);
    }

    virtual void onDraw(SkCanvas* canvas) {
        SkShader* shader = SkShader::CreateBitmapShader(fBitmap,
                                                        SkShader::kClamp_TileMode,
                                                        SkShader::kClamp_TileMode);
        SkPaint paint;
        paint.setShader(shader);
        // release the shader ref as the paint now holds a reference
        shader->unref();

        // draw the shader with a bitmap mask
        canvas->drawBitmap(fMask, 0, 0, &paint);
        canvas->drawBitmap(fMask, 30, 0, &paint);

           canvas->translate(0, 25);

        // draw the shader with standard geometry
           canvas->drawCircle(10, 10, 10, paint);
           canvas->drawCircle(40, 10, 10, paint); // no blue circle expected

           canvas->translate(0, 25);

        shader = SkShader::CreateBitmapShader(fMask,
                                              SkShader::kRepeat_TileMode,
                                              SkShader::kRepeat_TileMode);
        paint.setShader(shader);
        paint.setColor(SK_ColorRED);
        shader->unref();

           // draw the mask using the shader and a color
           canvas->drawRect(SkRect::MakeXYWH(0, 0, 20, 20), paint);
           canvas->drawRect(SkRect::MakeXYWH(30, 0, 20, 20), paint);
    }

private:
    SkBitmap fBitmap;
    SkBitmap fMask;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new BitmapShaderGM; }
static GMRegistry reg(MyFactory);

}
