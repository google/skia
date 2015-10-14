/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkBlurImageFilter.h"
#include "SkColorFilterImageFilter.h"
#include "SkModeColorFilter.h"
#include "SkMorphologyImageFilter.h"
#include "SkOffsetImageFilter.h"
#include "SkSurface.h"

static SkImage* make_image(SkCanvas* rootCanvas) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(100, 100);
    SkAutoTUnref<SkSurface> surface(rootCanvas->newSurface(info));
    if (!surface) {
        surface.reset(SkSurface::NewRaster(info));
    }

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorRED);
    surface->getCanvas()->drawCircle(50, 50, 50, paint);
    return surface->newImageSnapshot();
}

typedef SkImageFilter* (*ImageFilterFactory)();

// +[]{...} did not work on windows (VS)
// (ImageFilterFactory)[]{...} did not work on linux (gcc)
// hence this cast function
template <typename T> ImageFilterFactory IFCCast(T arg) { return arg; }

// Show the effect of localmatriximagefilter with various matrices, on various filters
class LocalMatrixImageFilterGM : public skiagm::GM {
public:
    LocalMatrixImageFilterGM() {}

protected:
    SkString onShortName() override {
        return SkString("localmatriximagefilter");
    }

    SkISize onISize() override {
        return SkISize::Make(640, 640);
    }

    static void show_image(SkCanvas* canvas, SkImage* image, SkImageFilter* filter) {
        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        SkRect r = SkRect::MakeIWH(image->width(), image->height()).makeOutset(SK_ScalarHalf,
                                                                               SK_ScalarHalf);
        canvas->drawRect(r, paint);

        paint.setStyle(SkPaint::kFill_Style);
        paint.setImageFilter(filter);
        canvas->drawImage(image, 0, 0, &paint);
    }

    void onDraw(SkCanvas* canvas) override {
        SkAutoTUnref<SkImage> image0(make_image(canvas));

        const ImageFilterFactory factories[] = {
            IFCCast([]{ return SkBlurImageFilter::Create(8, 8); }),
            IFCCast([]{ return SkDilateImageFilter::Create(8, 8); }),
            IFCCast([]{ return SkErodeImageFilter::Create(8, 8); }),
            IFCCast([]{ return SkOffsetImageFilter::Create(8, 8); }),
        };

        const SkMatrix matrices[] = {
            SkMatrix::MakeScale(SK_ScalarHalf, SK_ScalarHalf),
            SkMatrix::MakeScale(2, 2),
            SkMatrix::MakeTrans(10, 10)
        };

        const SkScalar spacer = image0->width() * 3.0f / 2;

        canvas->translate(40, 40);
        for (auto&& factory : factories) {
            SkAutoTUnref<SkImageFilter> filter(factory());

            canvas->save();
            show_image(canvas, image0, filter);
            for (const auto& matrix : matrices) {
                SkAutoTUnref<SkImageFilter> localFilter(filter->newWithLocalMatrix(matrix));
                canvas->translate(spacer, 0);
                show_image(canvas, image0, localFilter);
            }
            canvas->restore();
            canvas->translate(0, spacer);
        }
    }

private:
    typedef GM INHERITED;
};
DEF_GM( return new LocalMatrixImageFilterGM; )

