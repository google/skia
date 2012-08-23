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
#include "SkColorFilterImageFilter.h"
#include "SkMorphologyImageFilter.h"

#include "SkTestImageFilters.h"

///////////////////////////////////////////////////////////////////////////////

class ImageFiltersGraphGM : public skiagm::GM {
public:
    ImageFiltersGraphGM() : fInitialized(false) {}

protected:
#ifdef SK_BUILD_FOR_ANDROID
    // This test is currently broken when using pipe on Android
    virtual uint32_t onGetFlags() const SK_OVERRIDE {
        return this->INHERITED::onGetFlags() | skiagm::GM::kSkipPipe_Flag;
    }
#endif

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

    virtual SkISize onISize() { return SkISize::Make(500, 500); }

    virtual void onDraw(SkCanvas* canvas) {
        if (!fInitialized) {
            this->make_bitmap();
            fInitialized = true;
        }
        canvas->clear(0x00000000);

        SkAutoTUnref<SkImageFilter> bitmapSource(new SkBitmapSource(fBitmap));

        SkAutoTUnref<SkColorFilter> cf(SkColorFilter::CreateModeFilter(SK_ColorRED,
                                                     SkXfermode::kSrcIn_Mode));
        SkAutoTUnref<SkImageFilter> blur(new SkBlurImageFilter(4.0f, 4.0f, bitmapSource));
        SkAutoTUnref<SkImageFilter> erode(new SkErodeImageFilter(4, 4, blur));
        SkAutoTUnref<SkImageFilter> color(new SkColorFilterImageFilter(cf, erode));
        SkAutoTUnref<SkImageFilter> merge(new SkMergeImageFilter(blur, color));

        SkPaint paint;
        paint.setImageFilter(merge);
        canvas->drawPaint(paint);
    }

private:
    typedef GM INHERITED;
    SkBitmap fBitmap;
    bool fInitialized;
};

///////////////////////////////////////////////////////////////////////////////

static skiagm::GM* MyFactory(void*) { return new ImageFiltersGraphGM; }
static skiagm::GMRegistry reg(MyFactory);


