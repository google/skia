/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkBitmapSource.h"

namespace skiagm {

// This GM reproduces the issue in crbug.com/472795. The SkBitmapSource image
// is shifted for high quality mode between cpu and gpu.
class BitmapSourceGM : public GM {
public:
    BitmapSourceGM(const char* suffix, SkFilterQuality filter) : fSuffix(suffix), fFilter(filter) {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    SkString onShortName() override {
        SkString name("bitmapsrc2_");
        name.append(fSuffix);
        return name;
    }

    SkISize onISize() override { return SkISize::Make(256, 256); }

    // Create a bitmap with high frequency vertical stripes
    void onOnceBeforeDraw() override {
        static const SkPMColor gColors[] = {
            SK_ColorRED,     SK_ColorGRAY,
            SK_ColorGREEN,   SK_ColorGRAY,
            SK_ColorBLUE,    SK_ColorGRAY,
            SK_ColorCYAN,    SK_ColorGRAY,
            SK_ColorMAGENTA, SK_ColorGRAY,
            SK_ColorYELLOW,  SK_ColorGRAY,
            SK_ColorWHITE,   SK_ColorGRAY,
        };

        fBM.allocN32Pixels(kImageSize, kImageSize, true);

        SkCanvas canvas(fBM);

        int curColor = 0;

        for (int x = 0; x < kImageSize; x += 3) {
            SkRect r = SkRect::MakeXYWH(SkIntToScalar(x), SkIntToScalar(0), 
                                        SkIntToScalar(3), SkIntToScalar(kImageSize));
            SkPaint p;
            p.setColor(gColors[curColor]);
            canvas.drawRect(r, p);

            curColor = (curColor+1) % SK_ARRAY_COUNT(gColors);
        }
    }

    void onDraw(SkCanvas* canvas) override {
        SkRect srcRect = SkRect::MakeLTRB(0, 0,
                                          SkIntToScalar(kImageSize), SkIntToScalar(kImageSize));
        SkRect dstRect = SkRect::MakeLTRB(0.75f, 0.75f, 225.75f, 225.75f);

        SkAutoTUnref<SkImageFilter> filter(SkBitmapSource::Create(fBM, srcRect, dstRect, fFilter));

        SkPaint p;
        p.setImageFilter(filter);

        canvas->saveLayer(NULL, &p);
        canvas->restore();
    }

private:
    static const int kImageSize = 503;

    SkString fSuffix;
    SkFilterQuality fFilter;
    SkBitmap fBM;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return SkNEW_ARGS(BitmapSourceGM, ("none", kNone_SkFilterQuality) );   )
DEF_GM( return SkNEW_ARGS(BitmapSourceGM, ("low",  kLow_SkFilterQuality) );    )
DEF_GM( return SkNEW_ARGS(BitmapSourceGM, ("med",  kMedium_SkFilterQuality) ); )
DEF_GM( return SkNEW_ARGS(BitmapSourceGM, ("high", kHigh_SkFilterQuality) );   )

}
