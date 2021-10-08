/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"

namespace skiagm {

// This GM exercises HighQuality anisotropic filtering.
class AnisotropicGM : public GM {
public:
    AnisotropicGM() : fSampling(SkCubicResampler::Mitchell()) {
        this->setBGColor(0xFFCCCCCC);
    }

protected:

    SkString onShortName() override { return SkString("anisotropic_hq"); }

    SkISize onISize() override {
        return SkISize::Make(2*kImageSize + 3*kSpacer,
                             kNumVertImages*kImageSize + (kNumVertImages+1)*kSpacer);
    }

    // Create an image consisting of lines radiating from its center
    void onOnceBeforeDraw() override {
        constexpr int kNumLines = 100;
        constexpr SkScalar kAngleStep = 360.0f / kNumLines;
        constexpr int kInnerOffset = 10;

        auto info = SkImageInfo::MakeN32(kImageSize, kImageSize, kOpaque_SkAlphaType);
        auto surf = SkSurface::MakeRaster(info);
        auto canvas = surf->getCanvas();

        canvas->clear(SK_ColorWHITE);

        SkPaint p;
        p.setAntiAlias(true);

        SkScalar angle = 0.0f, sin, cos;

        canvas->translate(kImageSize/2.0f, kImageSize/2.0f);
        for (int i = 0; i < kNumLines; ++i, angle += kAngleStep) {
            sin = SkScalarSin(angle);
            cos = SkScalarCos(angle);
            canvas->drawLine(cos * kInnerOffset, sin * kInnerOffset,
                             cos * kImageSize/2, sin * kImageSize/2, p);
        }
        fImage = surf->makeImageSnapshot();
    }

    void draw(SkCanvas* canvas, int x, int y, int xSize, int ySize) {
        SkRect r = SkRect::MakeXYWH(SkIntToScalar(x), SkIntToScalar(y),
                                    SkIntToScalar(xSize), SkIntToScalar(ySize));
        canvas->drawImageRect(fImage, r, fSampling);
    }

    void onDraw(SkCanvas* canvas) override {
        SkScalar gScales[] = { 0.9f, 0.8f, 0.75f, 0.6f, 0.5f, 0.4f, 0.25f, 0.2f, 0.1f };

        SkASSERT(kNumVertImages-1 == (int)SK_ARRAY_COUNT(gScales)/2);

        // Minimize vertically
        for (int i = 0; i < (int)SK_ARRAY_COUNT(gScales); ++i) {
            int height = SkScalarFloorToInt(fImage->height() * gScales[i]);

            int yOff;
            if (i <= (int)SK_ARRAY_COUNT(gScales)/2) {
                yOff = kSpacer + i * (fImage->height() + kSpacer);
            } else {
                // Position the more highly squashed images with their less squashed counterparts
                yOff = (SK_ARRAY_COUNT(gScales) - i) * (fImage->height() + kSpacer) - height;
            }

            this->draw(canvas, kSpacer, yOff, fImage->width(), height);
        }

        // Minimize horizontally
        for (int i = 0; i < (int)SK_ARRAY_COUNT(gScales); ++i) {
            int width = SkScalarFloorToInt(fImage->width() * gScales[i]);

            int xOff, yOff;
            if (i <= (int)SK_ARRAY_COUNT(gScales)/2) {
                xOff = fImage->width() + 2*kSpacer;
                yOff = kSpacer + i * (fImage->height() + kSpacer);
            } else {
                // Position the more highly squashed images with their less squashed counterparts
                xOff = fImage->width() + 2*kSpacer + fImage->width() - width;
                yOff = kSpacer + (SK_ARRAY_COUNT(gScales) - i - 1) * (fImage->height() + kSpacer);
            }

            this->draw(canvas, xOff, yOff, width, fImage->height());
        }
    }

private:
    inline static constexpr int kImageSize     = 256;
    inline static constexpr int kSpacer        = 10;
    inline static constexpr int kNumVertImages = 5;

    sk_sp<SkImage>    fImage;
    SkSamplingOptions fSampling;

    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new AnisotropicGM;)
}  // namespace skiagm
