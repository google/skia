/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "SkArithmeticMode.h"
#include "SkBitmapSource.h"
#include "SkBlurImageFilter.h"
#include "SkColorFilter.h"
#include "SkColorFilterImageFilter.h"
#include "SkColorMatrixFilter.h"
#include "SkFlattenableBuffers.h"
#include "SkMergeImageFilter.h"
#include "SkMorphologyImageFilter.h"
#include "SkOnce.h"
#include "SkTestImageFilters.h"
#include "SkXfermodeImageFilter.h"

// More closely models how Blink's OffsetFilter works as of 10/23/13. SkOffsetImageFilter doesn't
// perform a draw and this one does.
class SimpleOffsetFilter : public SkImageFilter {
public:
    SimpleOffsetFilter(SkScalar dx, SkScalar dy, SkImageFilter* input)
    : SkImageFilter(input), fDX(dx), fDY(dy) {}

    virtual bool onFilterImage(Proxy* proxy, const SkBitmap& src, const SkMatrix& ctm,
                               SkBitmap* dst, SkIPoint* offset) SK_OVERRIDE {
        SkBitmap source = src;
        SkImageFilter* input = getInput(0);
        SkIPoint srcOffset = SkIPoint::Make(0, 0);
        if (NULL != input && !input->filterImage(proxy, src, ctm, &source, &srcOffset)) {
            return false;
        }

        SkIRect bounds;
        source.getBounds(&bounds);

        if (!this->applyCropRect(&bounds, ctm)) {
            return false;
        }

        SkAutoTUnref<SkBaseDevice> device(proxy->createDevice(bounds.width(), bounds.height()));
        SkCanvas canvas(device);
        SkPaint paint;
        paint.setXfermodeMode(SkXfermode::kSrc_Mode);
        canvas.drawBitmap(source, fDX - bounds.left(), fDY - bounds.top(), &paint);
        *dst = device->accessBitmap(false);
        offset->fX += bounds.left();
        offset->fY += bounds.top();
        return true;
    }

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SimpleOffsetFilter);

protected:
    explicit SimpleOffsetFilter(SkFlattenableReadBuffer& buffer)
    : SkImageFilter(1, buffer) {
        fDX = buffer.readScalar();
        fDY = buffer.readScalar();
    }

    virtual void flatten(SkFlattenableWriteBuffer& buffer) const SK_OVERRIDE {
        this->SkImageFilter::flatten(buffer);
        buffer.writeScalar(fDX);
        buffer.writeScalar(fDY);
    }

private:
    SkScalar fDX, fDY;
};

static void init_flattenable(int*) {
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SimpleOffsetFilter)
}

class ImageFiltersGraphGM : public skiagm::GM {
public:
    ImageFiltersGraphGM() : fInitialized(false) {
        int dummy;
        SK_DECLARE_STATIC_ONCE(once);
        SkOnce(&once, init_flattenable, &dummy);
    }

protected:
    virtual SkString onShortName() {
        return SkString("imagefiltersgraph");
    }

    void make_bitmap() {
        fBitmap.setConfig(SkBitmap::kARGB_8888_Config, 100, 100);
        fBitmap.allocPixels();
        SkBitmapDevice device(fBitmap);
        SkCanvas canvas(&device);
        canvas.clear(0x00000000);
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(0xFFFFFFFF);
        paint.setTextSize(SkIntToScalar(96));
        const char* str = "e";
        canvas.drawText(str, strlen(str), SkIntToScalar(20), SkIntToScalar(70), paint);
    }

    void drawClippedBitmap(SkCanvas* canvas, const SkBitmap& bitmap, const SkPaint& paint) {
        canvas->save();
        canvas->clipRect(SkRect::MakeXYWH(0, 0,
            SkIntToScalar(bitmap.width()), SkIntToScalar(bitmap.height())));
        canvas->drawBitmap(bitmap, 0, 0, &paint);
        canvas->restore();
    }

    virtual SkISize onISize() { return SkISize::Make(500, 150); }

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
            canvas->translate(SkIntToScalar(100), 0);
        }
        {
            SkAutoTUnref<SkImageFilter> morph(new SkDilateImageFilter(5, 5));

            SkScalar matrix[20] = { SK_Scalar1, 0, 0, 0, 0,
                                    0, SK_Scalar1, 0, 0, 0,
                                    0, 0, SK_Scalar1, 0, 0,
                                    0, 0, 0, 0.5f, 0 };

            SkAutoTUnref<SkColorFilter> matrixFilter(new SkColorMatrixFilter(matrix));
            SkAutoTUnref<SkImageFilter> colorMorph(SkColorFilterImageFilter::Create(matrixFilter, morph));
            SkAutoTUnref<SkXfermode> mode(SkXfermode::Create(SkXfermode::kSrcOver_Mode));
            SkAutoTUnref<SkImageFilter> blendColor(new SkXfermodeImageFilter(mode, colorMorph));

            SkPaint paint;
            paint.setImageFilter(blendColor);
            drawClippedBitmap(canvas, fBitmap, paint);
            canvas->translate(SkIntToScalar(100), 0);
        }
        {
            SkScalar matrix[20] = { SK_Scalar1, 0, 0, 0, 0,
                                    0, SK_Scalar1, 0, 0, 0,
                                    0, 0, SK_Scalar1, 0, 0,
                                    0, 0, 0, 0.5f, 0 };
            SkColorMatrixFilter matrixCF(matrix);
            SkAutoTUnref<SkImageFilter> matrixFilter(SkColorFilterImageFilter::Create(&matrixCF));
            SimpleOffsetFilter offsetFilter(SkIntToScalar(10), SkIntToScalar(10), matrixFilter);

            SkAutoTUnref<SkXfermode> arith(SkArithmeticMode::Create(0, SK_Scalar1, SK_Scalar1, 0));
            SkXfermodeImageFilter arithFilter(arith, matrixFilter, &offsetFilter);

            SkPaint paint;
            paint.setImageFilter(&arithFilter);
            drawClippedBitmap(canvas, fBitmap, paint);
            canvas->translate(SkIntToScalar(100), 0);
        }
        {
            SkAutoTUnref<SkImageFilter> blur(new SkBlurImageFilter(
              SkIntToScalar(10), SkIntToScalar(10)));

            SkAutoTUnref<SkXfermode> mode(SkXfermode::Create(SkXfermode::kSrcIn_Mode));
            SkImageFilter::CropRect cropRect(SkRect::MakeWH(SkIntToScalar(95), SkIntToScalar(100)));
            SkAutoTUnref<SkImageFilter> blend(new SkXfermodeImageFilter(mode, blur, NULL, &cropRect));

            SkPaint paint;
            paint.setImageFilter(blend);
            drawClippedBitmap(canvas, fBitmap, paint);
            canvas->translate(SkIntToScalar(100), 0);
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
