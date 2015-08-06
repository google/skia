/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapSource.h"
#include "SkTileImageFilter.h"
#include "gm.h"

static void create_circle_texture(SkBitmap* bm, SkColor color) {
    SkCanvas canvas(*bm);
    canvas.clear(0xFF000000);

    SkPaint paint;
    paint.setColor(color);
    paint.setStrokeWidth(3);
    paint.setStyle(SkPaint::kStroke_Style);

    canvas.drawCircle(SkScalarHalf(bm->width()), SkScalarHalf(bm->height()),
                      SkScalarHalf(bm->width()), paint);
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
        fRedBitmap.allocN32Pixels(kBitmapSize, kBitmapSize);
        create_circle_texture(&fRedBitmap, SK_ColorRED);

        fGreenBitmap.allocN32Pixels(kBitmapSize, kBitmapSize);
        create_circle_texture(&fGreenBitmap, SK_ColorGREEN);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorBLACK);

        {
            SkPaint p;

            SkRect bound = SkRect::MakeWH(SkIntToScalar(kWidth), SkIntToScalar(kHeight));
            SkAutoTUnref<SkBitmapSource> bms(SkBitmapSource::Create(fRedBitmap));
            SkAutoTUnref<SkTileImageFilter> tif(SkTileImageFilter::Create(
                            SkRect::MakeWH(SkIntToScalar(kBitmapSize), SkIntToScalar(kBitmapSize)),
                            SkRect::MakeWH(SkIntToScalar(kWidth), SkIntToScalar(kHeight)),
                            bms));
            p.setImageFilter(tif);

            canvas->saveLayer(&bound, &p);
            canvas->restore();
        }

        {
            SkPaint p2;

            SkRect bound2 = SkRect::MakeWH(SkIntToScalar(kBitmapSize), SkIntToScalar(kBitmapSize));

            SkAutoTUnref<SkTileImageFilter> tif2(SkTileImageFilter::Create(
                            SkRect::MakeWH(SkIntToScalar(kBitmapSize), SkIntToScalar(kBitmapSize)),
                            SkRect::MakeWH(SkIntToScalar(kBitmapSize), SkIntToScalar(kBitmapSize)),
                            NULL));
            p2.setImageFilter(tif2);

            canvas->translate(320, 320);
            canvas->saveLayer(&bound2, &p2);
            canvas->setMatrix(SkMatrix::I());

            SkRect bound3 = SkRect::MakeXYWH(320, 320,
                                             SkIntToScalar(kBitmapSize),
                                             SkIntToScalar(kBitmapSize));
            canvas->drawBitmapRect(fGreenBitmap, bound2, bound3, nullptr,
                                   SkCanvas::kStrict_SrcRectConstraint);
            canvas->restore();
        }
    }

private:
    static const int kWidth = 512;
    static const int kHeight = 512;
    static const int kBitmapSize = 64;

    SkBitmap fRedBitmap;
    SkBitmap fGreenBitmap;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return SkNEW(BigTileImageFilterGM); )

}
