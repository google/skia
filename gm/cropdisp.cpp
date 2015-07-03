/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkBitmapSource.h"
#include "SkColorFilter.h"
#include "SkColorFilterImageFilter.h"
#include "SkDisplacementMapEffect.h"
#include "SkTileImageFilter.h"
#include "SkXfermode.h"
#include "gm.h"

namespace skiagm {

// This tests the image filter graph:
//
//    BitmapSource1 -- all red 512x512
//        |
//    ColorFilterImageFilter -- with a 64x64 crop rect - makes the pixels green
//        |
//    TileImageFilter -- which tiles the 64x64 green pixels across 512x512
//        |
//        |                BitmapSource1 -- all red 512x512
//        | displacement   | color
//        |                |
//    DisplacementMapEffect -- this is only necessary to preserve the clip in the computed bounds
//                             TileImageFilter by itself bloats the bounds to include the src
//                             It has the TileImageFilter as the offset input.
//
// What was going on was that the clipRect being applied to the draw (64, 64, 512, 512)
// was eliminating the "displacement" chain due to the crop rect.
// This reproduces crbug/499499
class CroppedDisplacementGM : public GM {
public:
    CroppedDisplacementGM() { }

protected:

    SkString onShortName() override {
        return SkString("cropped-displacement");
    }

    SkISize onISize() override {
        return SkISize::Make(kWidth, kHeight);
    }

    void onOnceBeforeDraw() override {
        fRedBitmap.allocN32Pixels(kWidth, kHeight);
        SkCanvas canvas(fRedBitmap);
        canvas.clear(SK_ColorRED);
    }

    void onDraw(SkCanvas* canvas) override {

        SkPaint p;

        const SkRect smRect = SkRect::MakeWH(SkIntToScalar(kSmallSize), SkIntToScalar(kSmallSize));
        SkImageFilter::CropRect cr(smRect);

        SkAutoTUnref<SkBitmapSource> bms(SkBitmapSource::Create(fRedBitmap));
        SkAutoTUnref<SkColorFilter> cf(SkColorFilter::CreateModeFilter(SK_ColorGREEN,
                                                                       SkXfermode::kSrc_Mode));
        SkAutoTUnref<SkColorFilterImageFilter> cfif(SkColorFilterImageFilter::Create(cf, bms, &cr));

        SkAutoTUnref<SkTileImageFilter> tif(SkTileImageFilter::Create(
                        SkRect::MakeWH(SkIntToScalar(kSmallSize), SkIntToScalar(kSmallSize)),
                        SkRect::MakeWH(SkIntToScalar(kWidth), SkIntToScalar(kHeight)),
                        cfif));

        static const SkScalar kScale = 20.0f;

        SkAutoTUnref<SkDisplacementMapEffect> dif(SkDisplacementMapEffect::Create(
            SkDisplacementMapEffect::kB_ChannelSelectorType,
            SkDisplacementMapEffect::kB_ChannelSelectorType,
            kScale,
            tif, bms));

        p.setImageFilter(dif);

        canvas->clipRect(SkRect::MakeLTRB(kSmallSize+kScale/2.0f,
                                          kSmallSize+kScale/2.0f, 
                                          SkIntToScalar(kWidth), SkIntToScalar(kHeight)));
        canvas->saveLayer(NULL, &p);
        canvas->restore();
    }

private:
    static const int kWidth = 512;
    static const int kHeight = 512;
    static const int kSmallSize = 64;

    SkBitmap fRedBitmap;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return SkNEW(CroppedDisplacementGM); )

}
