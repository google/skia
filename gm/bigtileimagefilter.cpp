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

static sk_sp<SkImage> create_circle_texture(int size, SkColor color) {
    auto surface(SkSurface::MakeRasterN32Premul(size, size));
    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(0xFF000000);

    SkPaint paint;
    paint.setColor(color);
    paint.setStrokeWidth(3);
    paint.setStyle(SkPaint::kStroke_Style);

    canvas->drawCircle(SkScalarHalf(size), SkScalarHalf(size), SkScalarHalf(size), paint);

    return surface->makeImageSnapshot();
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
        fRedImage = create_circle_texture(kBitmapSize, SK_ColorRED);
        fGreenImage = create_circle_texture(kBitmapSize, SK_ColorGREEN);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorBLACK);

        {
            SkPaint p;

            const SkRect bound = SkRect::MakeIWH(kWidth, kHeight);
            sk_sp<SkImageFilter> imageSource(SkImageSource::Make(fRedImage));

            sk_sp<SkImageFilter> tif(SkTileImageFilter::Make(
                                                    SkRect::MakeIWH(kBitmapSize, kBitmapSize),
                                                    SkRect::MakeIWH(kWidth, kHeight),
                                                    std::move(imageSource)));

            p.setImageFilter(std::move(tif));

            canvas->saveLayer(&bound, &p);
            canvas->restore();
        }

        {
            SkPaint p2;

            const SkRect bound2 = SkRect::MakeIWH(kBitmapSize, kBitmapSize);

            sk_sp<SkImageFilter> tif(SkTileImageFilter::Make(
                                                        SkRect::MakeIWH(kBitmapSize, kBitmapSize),
                                                        SkRect::MakeIWH(kBitmapSize, kBitmapSize),
                                                        nullptr));

            p2.setImageFilter(std::move(tif));

            canvas->translate(320, 320);
            canvas->saveLayer(&bound2, &p2);
            canvas->setMatrix(SkMatrix::I());

            SkRect bound3 = SkRect::MakeXYWH(320, 320,
                                             SkIntToScalar(kBitmapSize),
                                             SkIntToScalar(kBitmapSize));
            canvas->drawImageRect(fGreenImage.get(), bound2, bound3, nullptr,
                                  SkCanvas::kStrict_SrcRectConstraint);
            canvas->restore();
        }
    }

private:
    static constexpr int kWidth = 512;
    static constexpr int kHeight = 512;
    static constexpr int kBitmapSize = 64;

    sk_sp<SkImage> fRedImage;
    sk_sp<SkImage> fGreenImage;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new BigTileImageFilterGM;)
}
