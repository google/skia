/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GrContext.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrContextPriv.h"

namespace skiagm {

static void draw_bm(SkBitmap* bm) {
    SkPaint bluePaint;
    bluePaint.setColor(SK_ColorBLUE);

    bm->allocN32Pixels(20, 20);
    bm->eraseColor(SK_ColorRED);

    SkCanvas canvas(*bm);
    canvas.drawCircle(10, 10, 5, bluePaint);
}

static void draw_mask(SkBitmap* bm) {
    SkPaint circlePaint;
    circlePaint.setColor(SK_ColorBLACK);

    bm->allocPixels(SkImageInfo::MakeA8(20, 20));
    bm->eraseColor(SK_ColorTRANSPARENT);

    SkCanvas canvas(*bm);
    canvas.drawCircle(10, 10, 10, circlePaint);
}

class BitmapShaderGM : public GM {

protected:
    void onOnceBeforeDraw() override {
        this->setBGColor(SK_ColorGRAY);
        draw_bm(&fBitmap);
        draw_mask(&fMask);
    }

    SkString onShortName() override {
        return SkString("bitmapshaders");
    }

    SkISize onISize() override {
        return SkISize::Make(150, 100);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;

        for (int i = 0; i < 2; i++) {
            SkMatrix s;
            s.reset();
            if (1 == i) {
                s.setScale(1.5f, 1.5f);
                s.postTranslate(2, 2);
            }

            canvas->save();
            paint.setShader(fBitmap.makeShader(&s));

            // draw the shader with a bitmap mask
            canvas->drawBitmap(fMask, 0, 0, &paint);
            // no blue circle expected (the bitmap shader's coordinates are aligned to CTM still)
            canvas->drawBitmap(fMask, 30, 0, &paint);

            canvas->translate(0, 25);

            canvas->drawCircle(10, 10, 10, paint);
            canvas->drawCircle(40, 10, 10, paint); // no blue circle expected

            canvas->translate(0, 25);

            // clear the shader, colorized by a solid color with a bitmap mask
            paint.setShader(nullptr);
            paint.setColor(SK_ColorGREEN);
            canvas->drawBitmap(fMask, 0, 0, &paint);
            canvas->drawBitmap(fMask, 30, 0, &paint);

            canvas->translate(0, 25);

            paint.setShader(fMask.makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat, &s));
            paint.setColor(SK_ColorRED);

            // draw the mask using the shader and a color
            canvas->drawRect(SkRect::MakeXYWH(0, 0, 20, 20), paint);
            canvas->drawRect(SkRect::MakeXYWH(30, 0, 20, 20), paint);
            canvas->restore();
            canvas->translate(60, 0);
        }
    }

private:
    SkBitmap fBitmap;
    SkBitmap fMask;

    typedef GM INHERITED;
};

DEF_SIMPLE_GM(hugebitmapshader, canvas, 100, 100) {
    SkPaint paint;
    SkBitmap bitmap;

    // The huge height will exceed GL_MAX_TEXTURE_SIZE. We test that the GL backend will at least
    // draw something with a default paint instead of drawing nothing.
    //
    // (See https://skia-review.googlesource.com/c/skia/+/73200)
    int bitmapW = 1;
    int bitmapH = 60000;
    if (auto* ctx = canvas->getGrContext()) {
        bitmapH = ctx->priv().caps()->maxTextureSize() + 1;
    }
    bitmap.setInfo(SkImageInfo::MakeA8(bitmapW, bitmapH), bitmapW);
    uint8_t* pixels = new uint8_t[bitmapH];
    for(int i = 0; i < bitmapH; ++i) {
        pixels[i] = i & 0xff;
    }
    bitmap.setPixels(pixels);

    paint.setShader(bitmap.makeShader(SkTileMode::kMirror, SkTileMode::kMirror));
    paint.setColor(SK_ColorRED);
    paint.setAntiAlias(true);
    canvas->drawCircle(50, 50, 50, paint);
    delete [] pixels;
}

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new BitmapShaderGM; )

}
