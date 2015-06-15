/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

namespace skiagm {

// This GM exercises HighQuality anisotropic filtering.
class AnisotropicGM : public GM {
public:
    AnisotropicGM() : fFilterQuality(kHigh_SkFilterQuality) {
        this->setBGColor(sk_tool_utils::color_to_565(0xFFCCCCCC));
    }

protected:

    SkString onShortName() override { return SkString("anisotropic_hq"); }

    SkISize onISize() override {
        return SkISize::Make(2*kImageSize + 3*kSpacer,
                             kNumVertImages*kImageSize + (kNumVertImages+1)*kSpacer);
    }

    // Create an image consisting of lines radiating from its center
    void onOnceBeforeDraw() override {
        static const int kNumLines = 100;
        static const SkScalar kAngleStep = 360.0f / kNumLines;
        static const int kInnerOffset = 10;

        fBM.allocN32Pixels(kImageSize, kImageSize, true);

        SkCanvas canvas(fBM);

        canvas.clear(SK_ColorWHITE);

        SkPaint p;
        p.setAntiAlias(true);

        SkScalar angle = 0.0f, sin, cos;

        canvas.translate(kImageSize/2.0f, kImageSize/2.0f);
        for (int i = 0; i < kNumLines; ++i, angle += kAngleStep) {
            sin = SkScalarSinCos(angle, &cos);
            canvas.drawLine(cos * kInnerOffset, sin * kInnerOffset,
                            cos * kImageSize/2, sin * kImageSize/2, p);
        }
    }

    void draw(SkCanvas* canvas, int x, int y, int xSize, int ySize) {
        SkRect r = SkRect::MakeXYWH(SkIntToScalar(x), SkIntToScalar(y),
                                    SkIntToScalar(xSize), SkIntToScalar(ySize));
        SkPaint p;
        p.setFilterQuality(fFilterQuality);
        canvas->drawBitmapRect(fBM, r, &p);
    }

    void onDraw(SkCanvas* canvas) override {
        SkScalar gScales[] = { 0.9f, 0.8f, 0.75f, 0.6f, 0.5f, 0.4f, 0.25f, 0.2f, 0.1f };
        
        SkASSERT(kNumVertImages-1 == (int)SK_ARRAY_COUNT(gScales)/2);

        // Minimize vertically
        for (int i = 0; i < (int)SK_ARRAY_COUNT(gScales); ++i) {
            int height = SkScalarFloorToInt(fBM.height() * gScales[i]);

            int yOff;
            if (i <= (int)SK_ARRAY_COUNT(gScales)/2) {
                yOff = kSpacer + i * (fBM.height() + kSpacer);
            } else {
                // Position the more highly squashed images with their less squashed counterparts
                yOff = (SK_ARRAY_COUNT(gScales) - i) * (fBM.height() + kSpacer) - height;
            }

            this->draw(canvas, kSpacer, yOff, fBM.width(), height);
        }

        // Minimize horizontally
        for (int i = 0; i < (int)SK_ARRAY_COUNT(gScales); ++i) {
            int width = SkScalarFloorToInt(fBM.width() * gScales[i]);

            int xOff, yOff;
            if (i <= (int)SK_ARRAY_COUNT(gScales)/2) {
                xOff = fBM.width() + 2*kSpacer;
                yOff = kSpacer + i * (fBM.height() + kSpacer);
            } else {
                // Position the more highly squashed images with their less squashed counterparts
                xOff = fBM.width() + 2*kSpacer + fBM.width() - width;
                yOff = kSpacer + (SK_ARRAY_COUNT(gScales) - i - 1) * (fBM.height() + kSpacer);
            }

            this->draw(canvas, xOff, yOff, width, fBM.height());
        }
    }

private:
    static const int kImageSize     = 256;
    static const int kSpacer        = 10;
    static const int kNumVertImages = 5;

    SkBitmap         fBM;
    SkFilterQuality  fFilterQuality;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return SkNEW(AnisotropicGM); )

}
