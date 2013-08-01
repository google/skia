/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "SkBitmapSource.h"
#include "SkBlurImageFilter.h"
#include "SkColorFilter.h"
#include "SkColorMatrixFilter.h"
#include "SkColorFilterImageFilter.h"
#include "SkMergeImageFilter.h"
#include "SkMorphologyImageFilter.h"
#include "SkXfermodeImageFilter.h"

#include "SkTestImageFilters.h"

///////////////////////////////////////////////////////////////////////////////

class ImageFiltersGraphGM : public skiagm::GM {
public:
    ImageFiltersGraphGM() : fInitialized(false) {}

protected:
    virtual SkString onShortName() {
        return SkString("imagefiltersgraph");
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

    virtual SkISize onISize() { return SkISize::Make(200, 100); }

    virtual void onDraw(SkCanvas* canvas) {
        if (!fInitialized) {
            this->make_bitmap();
            fInitialized = true;
        }
        canvas->clear(0x00000000);
        {
            SkAutoTUnref<SkImageFilter> bitmapSource(new SkBitmapSource(fBitmap));
            SkAutoTUnref<SkColorFilter> cf(SkColorFilter::CreateModeFilter(SK_ColorRED,
                                                         SkXfermode::kSrcIn_Mode));
            SkAutoTUnref<SkImageFilter> blur(new SkBlurImageFilter(4.0f, 4.0f, bitmapSource));
            SkAutoTUnref<SkImageFilter> erode(new SkErodeImageFilter(4, 4, blur));
            SkAutoTUnref<SkImageFilter> color(SkColorFilterImageFilter::Create(cf, erode));
            SkAutoTUnref<SkImageFilter> merge(new SkMergeImageFilter(blur, color));

            SkPaint paint;
            paint.setImageFilter(merge);
            canvas->drawPaint(paint);
        }
        {
            SkAutoTUnref<SkImageFilter> morph(new SkDilateImageFilter(5, 5));

            SkScalar matrix[20] = { SK_Scalar1, 0, 0, 0, 0,
                                    0, SK_Scalar1, 0, 0, 0,
                                    0, 0, SK_Scalar1, 0, 0,
                                    0, 0, 0, SkFloatToScalar(0.5f), 0 };

            SkAutoTUnref<SkColorFilter> matrixFilter(new SkColorMatrixFilter(matrix));
            SkAutoTUnref<SkImageFilter> colorMorph(SkColorFilterImageFilter::Create(matrixFilter, morph));
            SkAutoTUnref<SkXfermode> mode(SkXfermode::Create(SkXfermode::kSrcOver_Mode));
            SkAutoTUnref<SkImageFilter> blendColor(new SkXfermodeImageFilter(mode, colorMorph));

            SkPaint paint;
            paint.setImageFilter(blendColor);
            canvas->drawBitmap(fBitmap, 100, 0, &paint);
        }
    }

private:
    typedef GM INHERITED;
    SkBitmap fBitmap;
    bool fInitialized;
};

///////////////////////////////////////////////////////////////////////////////

static skiagm::GM* MyFactory(void*) { return new ImageFiltersGraphGM; }
static skiagm::GMRegistry reg(MyFactory);
