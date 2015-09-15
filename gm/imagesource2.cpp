/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkImage.h"
#include "SkImageSource.h"
#include "SkSurface.h"

namespace skiagm {

// This GM reproduces the issue in crbug.com/472795. The SkImageSource image
// is shifted for high quality mode between cpu and gpu.
class ImageSourceGM : public GM {
public:
    ImageSourceGM(const char* suffix, SkFilterQuality filter) : fSuffix(suffix), fFilter(filter) {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    SkString onShortName() override {
        SkString name("imagesrc2_");
        name.append(fSuffix);
        return name;
    }

    SkISize onISize() override { return SkISize::Make(256, 256); }

    // Create an image with high frequency vertical stripes
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

        SkAutoTUnref<SkSurface> surface(SkSurface::NewRasterN32Premul(kImageSize, kImageSize));
        SkCanvas* canvas = surface->getCanvas();

        int curColor = 0;

        for (int x = 0; x < kImageSize; x += 3) {
            SkRect r = SkRect::MakeXYWH(SkIntToScalar(x), SkIntToScalar(0), 
                                        SkIntToScalar(3), SkIntToScalar(kImageSize));
            SkPaint p;
            p.setColor(gColors[curColor]);
            canvas->drawRect(r, p);

            curColor = (curColor+1) % SK_ARRAY_COUNT(gColors);
        }

        fImage.reset(surface->newImageSnapshot());
    }

    void onDraw(SkCanvas* canvas) override {
        SkRect srcRect = SkRect::MakeLTRB(0, 0,
                                          SkIntToScalar(kImageSize), SkIntToScalar(kImageSize));
        SkRect dstRect = SkRect::MakeLTRB(0.75f, 0.75f, 225.75f, 225.75f);

        SkAutoTUnref<SkImageFilter> filter(
            SkImageSource::Create(fImage, srcRect, dstRect, fFilter));

        SkPaint p;
        p.setImageFilter(filter);

        canvas->saveLayer(nullptr, &p);
        canvas->restore();
    }

private:
    static const int kImageSize = 503;

    SkString fSuffix;
    SkFilterQuality fFilter;
    SkAutoTUnref<SkImage> fImage;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ImageSourceGM("none", kNone_SkFilterQuality);)
DEF_GM(return new ImageSourceGM("low", kLow_SkFilterQuality);)
DEF_GM(return new ImageSourceGM("med", kMedium_SkFilterQuality);)
DEF_GM(return new ImageSourceGM("high", kHigh_SkFilterQuality);)
}
