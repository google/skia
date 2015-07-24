/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "SkArithmeticMode.h"
#include "SkDevice.h"
#include "SkBitmapSource.h"
#include "SkBlurImageFilter.h"
#include "SkColorFilter.h"
#include "SkColorFilterImageFilter.h"
#include "SkColorMatrixFilter.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkMergeImageFilter.h"
#include "SkMorphologyImageFilter.h"
#include "SkTestImageFilters.h"
#include "SkXfermodeImageFilter.h"

// More closely models how Blink's OffsetFilter works as of 10/23/13. SkOffsetImageFilter doesn't
// perform a draw and this one does.
class SimpleOffsetFilter : public SkImageFilter {
public:
    class Registrar {
    public:
        Registrar() {
            SkFlattenable::Register("SimpleOffsetFilter",
                                    SimpleOffsetFilter::CreateProc,
                                    SimpleOffsetFilter::GetFlattenableType());
        }
    };
    static SkImageFilter* Create(SkScalar dx, SkScalar dy, SkImageFilter* input) {
        return SkNEW_ARGS(SimpleOffsetFilter, (dx, dy, input));
    }

    virtual bool onFilterImage(Proxy* proxy, const SkBitmap& src, const Context& ctx,
                               SkBitmap* dst, SkIPoint* offset) const override {
        SkBitmap source = src;
        SkImageFilter* input = getInput(0);
        SkIPoint srcOffset = SkIPoint::Make(0, 0);
        if (input && !input->filterImage(proxy, src, ctx, &source, &srcOffset)) {
            return false;
        }

        SkIRect bounds;
        if (!this->applyCropRect(ctx, proxy, source, &srcOffset, &bounds, &source)) {
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

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SimpleOffsetFilter);

protected:
    void flatten(SkWriteBuffer& buffer) const override {
        this->INHERITED::flatten(buffer);
        buffer.writeScalar(fDX);
        buffer.writeScalar(fDY);
    }

private:
    SimpleOffsetFilter(SkScalar dx, SkScalar dy, SkImageFilter* input)
        : SkImageFilter(1, &input), fDX(dx), fDY(dy) {}

    SkScalar fDX, fDY;

    typedef SkImageFilter INHERITED;
};

static SimpleOffsetFilter::Registrar gReg;

SkFlattenable* SimpleOffsetFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    SkScalar dx = buffer.readScalar();
    SkScalar dy = buffer.readScalar();
    return Create(dx, dy, common.getInput(0));
}

#ifndef SK_IGNORE_TO_STRING
void SimpleOffsetFilter::toString(SkString* str) const {
    str->appendf("SimpleOffsetFilter: (");
    str->append(")");
}
#endif

class ImageFiltersGraphGM : public skiagm::GM {
public:
    ImageFiltersGraphGM() {}

protected:

    virtual SkString onShortName() {
        return SkString("imagefiltersgraph");
    }

    void make_bitmap() {
        fBitmap.allocN32Pixels(100, 100);
        SkCanvas canvas(fBitmap);
        canvas.clear(SK_ColorTRANSPARENT);
        SkPaint paint;
        paint.setAntiAlias(true);
        sk_tool_utils::set_portable_typeface(&paint);
        paint.setColor(SK_ColorWHITE);
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

    virtual void onOnceBeforeDraw() {
        this->make_bitmap();
    }

    virtual void onDraw(SkCanvas* canvas) {
        canvas->clear(SK_ColorBLACK);
        {
            SkAutoTUnref<SkImageFilter> bitmapSource(SkBitmapSource::Create(fBitmap));
            SkAutoTUnref<SkColorFilter> cf(SkColorFilter::CreateModeFilter(SK_ColorRED,
                                                         SkXfermode::kSrcIn_Mode));
            SkAutoTUnref<SkImageFilter> blur(SkBlurImageFilter::Create(4.0f, 4.0f, bitmapSource));
            SkAutoTUnref<SkImageFilter> erode(SkErodeImageFilter::Create(4, 4, blur));
            SkAutoTUnref<SkImageFilter> color(SkColorFilterImageFilter::Create(cf, erode));
            SkAutoTUnref<SkImageFilter> merge(SkMergeImageFilter::Create(blur, color));

            SkPaint paint;
            paint.setImageFilter(merge);
            canvas->drawPaint(paint);
            canvas->translate(SkIntToScalar(100), 0);
        }
        {
            SkAutoTUnref<SkImageFilter> morph(SkDilateImageFilter::Create(5, 5));

            SkScalar matrix[20] = { SK_Scalar1, 0, 0, 0, 0,
                                    0, SK_Scalar1, 0, 0, 0,
                                    0, 0, SK_Scalar1, 0, 0,
                                    0, 0, 0, 0.5f, 0 };

            SkAutoTUnref<SkColorFilter> matrixFilter(SkColorMatrixFilter::Create(matrix));
            SkAutoTUnref<SkImageFilter> colorMorph(SkColorFilterImageFilter::Create(matrixFilter, morph));
            SkAutoTUnref<SkXfermode> mode(SkXfermode::Create(SkXfermode::kSrcOver_Mode));
            SkAutoTUnref<SkImageFilter> blendColor(SkXfermodeImageFilter::Create(mode, colorMorph));

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
            SkAutoTUnref<SkColorMatrixFilter> matrixCF(SkColorMatrixFilter::Create(matrix));
            SkAutoTUnref<SkImageFilter> matrixFilter(SkColorFilterImageFilter::Create(matrixCF));
            SkAutoTUnref<SkImageFilter> offsetFilter(
                SimpleOffsetFilter::Create(10.0f, 10.f, matrixFilter));

            SkAutoTUnref<SkXfermode> arith(SkArithmeticMode::Create(0, SK_Scalar1, SK_Scalar1, 0));
            SkAutoTUnref<SkXfermodeImageFilter> arithFilter(
                SkXfermodeImageFilter::Create(arith, matrixFilter, offsetFilter));

            SkPaint paint;
            paint.setImageFilter(arithFilter);
            drawClippedBitmap(canvas, fBitmap, paint);
            canvas->translate(SkIntToScalar(100), 0);
        }
        {
            SkAutoTUnref<SkImageFilter> blur(SkBlurImageFilter::Create(
              SkIntToScalar(10), SkIntToScalar(10)));

            SkAutoTUnref<SkXfermode> mode(SkXfermode::Create(SkXfermode::kSrcIn_Mode));
            SkImageFilter::CropRect cropRect(SkRect::MakeWH(SkIntToScalar(95), SkIntToScalar(100)));
            SkAutoTUnref<SkImageFilter> blend(
                SkXfermodeImageFilter::Create(mode, blur, NULL, &cropRect));

            SkPaint paint;
            paint.setImageFilter(blend);
            drawClippedBitmap(canvas, fBitmap, paint);
            canvas->translate(SkIntToScalar(100), 0);
        }
        {
            // Test that crop offsets are absolute, not relative to the parent's crop rect.
            SkAutoTUnref<SkColorFilter> cf1(SkColorFilter::CreateModeFilter(SK_ColorBLUE,
                                                                            SkXfermode::kSrcIn_Mode));
            SkAutoTUnref<SkColorFilter> cf2(SkColorFilter::CreateModeFilter(SK_ColorGREEN,
                                                                            SkXfermode::kSrcIn_Mode));
            SkImageFilter::CropRect outerRect(SkRect::MakeXYWH(SkIntToScalar(10), SkIntToScalar(10),
                                                               SkIntToScalar(80), SkIntToScalar(80)));
            SkImageFilter::CropRect innerRect(SkRect::MakeXYWH(SkIntToScalar(20), SkIntToScalar(20),
                                                               SkIntToScalar(60), SkIntToScalar(60)));
            SkAutoTUnref<SkImageFilter> color1(SkColorFilterImageFilter::Create(cf1, NULL, &outerRect));
            SkAutoTUnref<SkImageFilter> color2(SkColorFilterImageFilter::Create(cf2, color1, &innerRect));

            SkPaint paint;
            paint.setImageFilter(color2);
            paint.setColor(SK_ColorRED);
            canvas->drawRect(SkRect::MakeXYWH(0, 0, 100, 100), paint);
            canvas->translate(SkIntToScalar(100), 0);
        }
    }

private:
    typedef GM INHERITED;
    SkBitmap fBitmap;
};

///////////////////////////////////////////////////////////////////////////////

static skiagm::GM* MyFactory(void*) { return new ImageFiltersGraphGM; }
static skiagm::GMRegistry reg(MyFactory);
