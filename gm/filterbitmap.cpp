/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkGradientShader.h"

#include "SkTypeface.h"
#include "SkImageDecoder.h"
static void load_bm(SkBitmap* bm) {
//    SkImageDecoder::DecodeFile("/skia/trunk/books.jpg", bm);

    bm->setConfig(SkBitmap::kARGB_8888_Config, 160, 120);
    bm->allocPixels();
    SkCanvas canvas(*bm);
    canvas.drawColor(SK_ColorWHITE);

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setSubpixelText(true);
    paint.setTextSize(17);

    paint.setTypeface(SkTypeface::CreateFromName("Times", SkTypeface::kNormal))->unref();
    canvas.drawText("Hamburgefons", 12, 10, 25, paint);
    paint.setTypeface(SkTypeface::CreateFromName("Times", SkTypeface::kItalic))->unref();
    canvas.drawText("Hamburgefons", 12, 10, 50, paint);
    paint.setTypeface(SkTypeface::CreateFromName("Times", SkTypeface::kBold))->unref();
    canvas.drawText("Hamburgefons", 12, 10, 75, paint);
    paint.setTypeface(SkTypeface::CreateFromName("Times", SkTypeface::kBoldItalic))->unref();
    canvas.drawText("Hamburgefons", 12, 10, 100, paint);
}

static SkSize computeSize(const SkBitmap& bm, const SkMatrix& mat) {
    SkRect bounds = { 0, 0, bm.width(), bm.height() };
    mat.mapRect(&bounds);
    return SkSize::Make(bounds.width(), bounds.height());
}

static void draw_col(SkCanvas* canvas, const SkBitmap& bm, const SkMatrix& mat,
                     SkScalar dx) {
    SkPaint paint;

    SkAutoCanvasRestore acr(canvas, true);

    canvas->drawBitmapMatrix(bm, mat, &paint);

    paint.setFilterBitmap(true);
    canvas->translate(dx, 0);
    canvas->drawBitmapMatrix(bm, mat, &paint);

    paint.setFlags(paint.getFlags() | SkPaint::kBicubicFilterBitmap_Flag);
    canvas->translate(dx, 0);
    canvas->drawBitmapMatrix(bm, mat, &paint);
}

class FilterBitmapGM : public skiagm::GM {
    bool fOnce;
    void init() {
        if (fOnce) {
            return;
        }
        fOnce = true;
        load_bm(&fBM);

        SkScalar cx = SkScalarHalf(fBM.width());
        SkScalar cy = SkScalarHalf(fBM.height());
        SkScalar scale = 1.6f;

        fMatrix[0].setScale(scale, scale);
        fMatrix[1].setRotate(30, cx, cy); fMatrix[1].postScale(scale, scale);
    }

public:
    SkBitmap    fBM;
    SkMatrix    fMatrix[2];

    FilterBitmapGM() : fOnce(false) {
        this->setBGColor(0xFFDDDDDD);
    }

protected:
    virtual SkString onShortName() SK_OVERRIDE {
        return SkString("filterbitmap");
    }

    virtual SkISize onISize() SK_OVERRIDE {
        return SkISize::Make(920, 480);
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        this->init();

        canvas->translate(10, 10);
        for (size_t i = 0; i < SK_ARRAY_COUNT(fMatrix); ++i) {
            SkSize size = computeSize(fBM, fMatrix[i]);
            size.fWidth += 20;
            size.fHeight += 20;

            draw_col(canvas, fBM, fMatrix[i], size.fWidth);
            canvas->translate(0, size.fHeight);
        }
    }

private:
    typedef skiagm::GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new FilterBitmapGM; )

