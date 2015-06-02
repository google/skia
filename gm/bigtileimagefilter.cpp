/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapSource.h"
#include "SkTileImageFilter.h"
#include "gm.h"

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
        fBitmap.allocN32Pixels(kBitmapSize, kBitmapSize);

        SkCanvas canvas(fBitmap);
        canvas.clear(0xFF000000);

        SkPaint paint;
        paint.setColor(SK_ColorRED);
        paint.setStrokeWidth(3);
        paint.setStyle(SkPaint::kStroke_Style);

        canvas.drawCircle(SkScalarHalf(kBitmapSize), SkScalarHalf(kBitmapSize),
                          SkScalarHalf(kBitmapSize), paint);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorBLACK);

        SkPaint p;

        SkAutoTUnref<SkBitmapSource> bms(SkBitmapSource::Create(fBitmap));
        SkAutoTUnref<SkTileImageFilter> tif(SkTileImageFilter::Create(
                            SkRect::MakeWH(SkIntToScalar(kBitmapSize), SkIntToScalar(kBitmapSize)),
                            SkRect::MakeWH(SkIntToScalar(kWidth), SkIntToScalar(kHeight)),
                            bms));
        p.setImageFilter(tif);

        SkRect bound = SkRect::MakeWH(SkIntToScalar(kWidth), SkIntToScalar(kHeight));
        canvas->saveLayer(&bound, &p);
        canvas->restore();
    }

private:
    static const int kWidth = 512;
    static const int kHeight = 512;
    static const int kBitmapSize = 64;

    SkBitmap fBitmap;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return SkNEW(BigTileImageFilterGM); )

}
