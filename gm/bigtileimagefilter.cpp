/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImageSource.h"
#include "SkSurface.h"
#include "SkTileImageFilter.h"
#include "gm.h"

static SkImage* create_circle_texture(int size, SkColor color) {
    SkAutoTUnref<SkSurface> surface(SkSurface::NewRasterN32Premul(size, size));
    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(0xFF000000);

    SkPaint paint;
    paint.setColor(color);
    paint.setStrokeWidth(3);
    paint.setStyle(SkPaint::kStroke_Style);

    canvas->drawCircle(SkScalarHalf(size), SkScalarHalf(size), SkScalarHalf(size), paint);

    return surface->newImageSnapshot();
}

namespace skiagm {

class BigTileImageFilterGM : public GM {
public:
    BigTileImageFilterGM() {
        this->setBGColor(0xFF000000);
    }

protected:

    SkString onShortName() override {
        return SkString("bigtileimagefilter");
    }

    SkISize onISize() override{
        return SkISize::Make(kWidth, kHeight);
    }

    void onOnceBeforeDraw() override {
        fRedImage.reset(create_circle_texture(kBitmapSize, SK_ColorRED));
        fGreenImage.reset(create_circle_texture(kBitmapSize, SK_ColorGREEN));
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorBLACK);

        {
            SkPaint p;

            SkRect bound = SkRect::MakeWH(SkIntToScalar(kWidth), SkIntToScalar(kHeight));
            SkAutoTUnref<SkImageFilter> imageSource(SkImageSource::Create(fRedImage));
            SkAutoTUnref<SkImageFilter> tif(SkTileImageFilter::Create(
                            SkRect::MakeWH(SkIntToScalar(kBitmapSize), SkIntToScalar(kBitmapSize)),
                            SkRect::MakeWH(SkIntToScalar(kWidth), SkIntToScalar(kHeight)),
                            imageSource));
            p.setImageFilter(tif);

            canvas->saveLayer(&bound, &p);
            canvas->restore();
        }

        {
            SkPaint p2;

            SkRect bound2 = SkRect::MakeWH(SkIntToScalar(kBitmapSize), SkIntToScalar(kBitmapSize));

            SkAutoTUnref<SkImageFilter> tif2(SkTileImageFilter::Create(
                            SkRect::MakeWH(SkIntToScalar(kBitmapSize), SkIntToScalar(kBitmapSize)),
                            SkRect::MakeWH(SkIntToScalar(kBitmapSize), SkIntToScalar(kBitmapSize)),
                            nullptr));
            p2.setImageFilter(tif2);

            canvas->translate(320, 320);
            canvas->saveLayer(&bound2, &p2);
            canvas->setMatrix(SkMatrix::I());

            SkRect bound3 = SkRect::MakeXYWH(320, 320,
                                             SkIntToScalar(kBitmapSize),
                                             SkIntToScalar(kBitmapSize));
            canvas->drawImageRect(fGreenImage, bound2, bound3, nullptr,
                                  SkCanvas::kStrict_SrcRectConstraint);
            canvas->restore();
        }
    }

private:
    static const int kWidth = 512;
    static const int kHeight = 512;
    static const int kBitmapSize = 64;

    SkAutoTUnref<SkImage> fRedImage;
    SkAutoTUnref<SkImage> fGreenImage;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new BigTileImageFilterGM;)
}
