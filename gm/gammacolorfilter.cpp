/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkGammaColorFilter.h"
#include "SkImage.h"

// Fill a width x height block with a horizontal ramp from 0 to 255
static void draw_grey_ramp(SkCanvas* canvas, int width, int height, int numSteps) {
    int greyPerStep = SkScalarRoundToInt(255.0f / numSteps);
    int widthPerStep = SkScalarRoundToInt(width / SkIntToScalar(numSteps));

    SkIRect rect = SkIRect::MakeWH(widthPerStep, height);

    SkPaint paint;
    int grey = 0;
    int x = 0;
    for (int i = 0; i < numSteps-1; ++i) {
        paint.setColor(SkColorSetARGB(255, grey, grey, grey));

        rect.offsetTo(x, 0);
        canvas->drawRect(SkRect::Make(rect), paint);

        x += widthPerStep;
        grey += greyPerStep;
    }

    paint.setColor(SK_ColorWHITE);
    rect.setLTRB(x, 0, width, height);
    canvas->drawRect(SkRect::Make(rect), paint);
}

static sk_sp<SkImage> create_grey_ramp(int width, int height, int numSteps) {
    SkBitmap bm;
    bm.allocN32Pixels(width, height);
    SkCanvas canvas(bm);
    canvas.clear(0x0);

    draw_grey_ramp(&canvas, width, height, numSteps);

    return SkImage::MakeFromBitmap(bm);
}

namespace skiagm {

class GammaColorFilterGM : public GM {
public:
    GammaColorFilterGM() {
        this->setBGColor(SK_ColorBLACK);
    }

protected:

    SkString onShortName() override {
        return SkString("gammacolorfilter");
    }

    SkISize onISize() override {
        return SkISize::Make(4 * kCellWidth, kCellHeight);
    }

    void onDraw(SkCanvas* canvas) override {
        GrRenderTargetContext* renderTargetContext =
            canvas->internal_private_accessTopLayerRenderTargetContext();
        if (!renderTargetContext) {
            skiagm::GM::DrawGpuOnlyMessage(canvas);
            return;
        }

        sk_sp<SkImage> image(create_grey_ramp(kCellWidth, kCellHeight/2, kNumGreySteps));

        // Leftmost is a non-gamma pair
        draw_grey_ramp(canvas, kCellWidth, kCellHeight/2, kNumGreySteps);
        canvas->drawImage(image, 0, kCellHeight/2);
        canvas->translate(SkIntToScalar(image->width()), 0);

        for (auto gamma : { 1.0f, 1.0f / 1.8f, 1.0f / 2.2f }) {
            SkPaint paint;
            paint.setColorFilter(SkGammaColorFilter::Make(gamma));

            draw_grey_ramp(canvas, kCellWidth, kCellHeight/2, kNumGreySteps);
            canvas->drawImage(image, 0, kCellHeight/2, &paint);
            canvas->translate(SkIntToScalar(image->width()), 0);
        }
    }

private:
    static constexpr int kCellWidth = 64;
    static constexpr int kCellHeight = 64;
    static constexpr int kNumGreySteps = 16;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new GammaColorFilterGM;)
}
