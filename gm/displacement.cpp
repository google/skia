/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDisplacementMapEffect.h"
#include "SkImage.h"
#include "SkImageSource.h"
#include "gm.h"
#include "sk_tool_utils.h"

namespace skiagm {

class DisplacementMapGM : public GM {
public:
    DisplacementMapGM() {
        this->setBGColor(0xFF000000);
    }

protected:
    SkString onShortName() override {
        return SkString("displacement");
    }

    void onOnceBeforeDraw() override {
        fBitmap = sk_tool_utils::create_string_bitmap(80, 80, 0xFF884422, 15, 55, 96, "g");

        SkColor c1 = sk_tool_utils::color_to_565(0xFF244484);
        SkColor c2 = sk_tool_utils::color_to_565(0xFF804020);

        fCheckerboard = SkImage::MakeFromBitmap(
            sk_tool_utils::create_checkerboard_bitmap(80, 80, c1, c2, 8));
        fSmall = SkImage::MakeFromBitmap(
            sk_tool_utils::create_checkerboard_bitmap(64, 64, c1, c2, 8));
        fLarge = SkImage::MakeFromBitmap(
            sk_tool_utils::create_checkerboard_bitmap(96, 96, c1, c2, 8));
        fLargeW =
            SkImage::MakeFromBitmap(sk_tool_utils::create_checkerboard_bitmap(96, 64, c1, c2, 8));
        fLargeH = SkImage::MakeFromBitmap(
            sk_tool_utils::create_checkerboard_bitmap(64, 96, c1, c2, 8));
    }

    SkISize onISize() override {
        return SkISize::Make(600, 500);
    }

    void drawClippedBitmap(SkCanvas* canvas, int x, int y, const SkPaint& paint) const {
        canvas->save();
        canvas->translate(SkIntToScalar(x), SkIntToScalar(y));
        canvas->clipRect(SkRect::MakeWH(SkIntToScalar(fBitmap.width()), SkIntToScalar(fBitmap.height())));
        canvas->drawBitmap(fBitmap, 0, 0, &paint);
        canvas->restore();
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorBLACK);
        SkPaint paint;
        sk_sp<SkImageFilter> displ(SkImageSource::Make(fCheckerboard));
        paint.setImageFilter(SkDisplacementMapEffect::Make(
            SkDisplacementMapEffect::kR_ChannelSelectorType,
            SkDisplacementMapEffect::kG_ChannelSelectorType, 0.0f, displ, nullptr));
        this->drawClippedBitmap(canvas, 0, 0, paint);
        paint.setImageFilter(SkDisplacementMapEffect::Make(
            SkDisplacementMapEffect::kB_ChannelSelectorType,
            SkDisplacementMapEffect::kA_ChannelSelectorType, 16.0f, displ, nullptr));
        this->drawClippedBitmap(canvas, 100, 0, paint);
        paint.setImageFilter(SkDisplacementMapEffect::Make(
            SkDisplacementMapEffect::kR_ChannelSelectorType,
            SkDisplacementMapEffect::kB_ChannelSelectorType, 32.0f, displ, nullptr));
        this->drawClippedBitmap(canvas, 200, 0, paint);
        paint.setImageFilter(SkDisplacementMapEffect::Make(
            SkDisplacementMapEffect::kG_ChannelSelectorType,
            SkDisplacementMapEffect::kA_ChannelSelectorType, 48.0f, displ, nullptr));
        this->drawClippedBitmap(canvas, 300, 0, paint);
        paint.setImageFilter(SkDisplacementMapEffect::Make(
            SkDisplacementMapEffect::kR_ChannelSelectorType,
            SkDisplacementMapEffect::kA_ChannelSelectorType, 64.0f, displ, nullptr));
        this->drawClippedBitmap(canvas, 400, 0, paint);

        paint.setImageFilter(SkDisplacementMapEffect::Make(
            SkDisplacementMapEffect::kR_ChannelSelectorType,
            SkDisplacementMapEffect::kG_ChannelSelectorType, 40.0f, displ, nullptr));
        this->drawClippedBitmap(canvas, 0, 100, paint);
        paint.setImageFilter(SkDisplacementMapEffect::Make(
            SkDisplacementMapEffect::kB_ChannelSelectorType,
            SkDisplacementMapEffect::kA_ChannelSelectorType, 40.0f, displ, nullptr));
        this->drawClippedBitmap(canvas, 100, 100, paint);
        paint.setImageFilter(SkDisplacementMapEffect::Make(
            SkDisplacementMapEffect::kR_ChannelSelectorType,
            SkDisplacementMapEffect::kB_ChannelSelectorType, 40.0f, displ, nullptr));
        this->drawClippedBitmap(canvas, 200, 100, paint);
        paint.setImageFilter(SkDisplacementMapEffect::Make(
            SkDisplacementMapEffect::kG_ChannelSelectorType,
            SkDisplacementMapEffect::kA_ChannelSelectorType, 40.0f, displ, nullptr));
        this->drawClippedBitmap(canvas, 300, 100, paint);
        paint.setImageFilter(SkDisplacementMapEffect::Make(
            SkDisplacementMapEffect::kR_ChannelSelectorType,
            SkDisplacementMapEffect::kA_ChannelSelectorType, 40.0f, displ, nullptr));
        this->drawClippedBitmap(canvas, 400, 100, paint);

        SkImageFilter::CropRect cropRect(SkRect::MakeXYWH(30, 30, 40, 40));
        paint.setImageFilter(SkDisplacementMapEffect::Make(
            SkDisplacementMapEffect::kR_ChannelSelectorType,
            SkDisplacementMapEffect::kG_ChannelSelectorType,
            0.0f, displ, nullptr, &cropRect));
        this->drawClippedBitmap(canvas, 0, 200, paint);
        paint.setImageFilter(SkDisplacementMapEffect::Make(
            SkDisplacementMapEffect::kB_ChannelSelectorType,
            SkDisplacementMapEffect::kA_ChannelSelectorType,
            16.0f, displ, nullptr, &cropRect));
        this->drawClippedBitmap(canvas, 100, 200, paint);
        paint.setImageFilter(SkDisplacementMapEffect::Make(
            SkDisplacementMapEffect::kR_ChannelSelectorType,
            SkDisplacementMapEffect::kB_ChannelSelectorType,
            32.0f, displ, nullptr, &cropRect));
        this->drawClippedBitmap(canvas, 200, 200, paint);
        paint.setImageFilter(SkDisplacementMapEffect::Make(
            SkDisplacementMapEffect::kG_ChannelSelectorType,
            SkDisplacementMapEffect::kA_ChannelSelectorType,
            48.0f, displ, nullptr, &cropRect));
        this->drawClippedBitmap(canvas, 300, 200, paint);
        paint.setImageFilter(SkDisplacementMapEffect::Make(
            SkDisplacementMapEffect::kR_ChannelSelectorType,
            SkDisplacementMapEffect::kA_ChannelSelectorType,
            64.0f, displ, nullptr, &cropRect));
        this->drawClippedBitmap(canvas, 400, 200, paint);

        paint.setImageFilter(SkDisplacementMapEffect::Make(
            SkDisplacementMapEffect::kR_ChannelSelectorType,
            SkDisplacementMapEffect::kG_ChannelSelectorType,
            40.0f, displ, nullptr, &cropRect));
        this->drawClippedBitmap(canvas, 0, 300, paint);
        paint.setImageFilter(SkDisplacementMapEffect::Make(
            SkDisplacementMapEffect::kB_ChannelSelectorType,
            SkDisplacementMapEffect::kA_ChannelSelectorType,
            40.0f, displ, nullptr, &cropRect));
        this->drawClippedBitmap(canvas, 100, 300, paint);
        paint.setImageFilter(SkDisplacementMapEffect::Make(
            SkDisplacementMapEffect::kR_ChannelSelectorType,
            SkDisplacementMapEffect::kB_ChannelSelectorType,
            40.0f, displ, nullptr, &cropRect));
        this->drawClippedBitmap(canvas, 200, 300, paint);
        paint.setImageFilter(SkDisplacementMapEffect::Make(
            SkDisplacementMapEffect::kG_ChannelSelectorType,
            SkDisplacementMapEffect::kA_ChannelSelectorType,
            40.0f, displ, nullptr, &cropRect));
        this->drawClippedBitmap(canvas, 300, 300, paint);
        paint.setImageFilter(SkDisplacementMapEffect::Make(
            SkDisplacementMapEffect::kR_ChannelSelectorType,
            SkDisplacementMapEffect::kA_ChannelSelectorType,
            40.0f, displ, nullptr, &cropRect));
        this->drawClippedBitmap(canvas, 400, 300, paint);

        // Test for negative scale.
        paint.setImageFilter(SkDisplacementMapEffect::Make(
            SkDisplacementMapEffect::kG_ChannelSelectorType,
            SkDisplacementMapEffect::kA_ChannelSelectorType,
            -40.0f, displ, nullptr));
        this->drawClippedBitmap(canvas, 500, 0, paint);

        // Tests for images of different sizes
        displ = SkImageSource::Make(fSmall);
        paint.setImageFilter(SkDisplacementMapEffect::Make(
            SkDisplacementMapEffect::kR_ChannelSelectorType,
            SkDisplacementMapEffect::kG_ChannelSelectorType,
            40.0f, std::move(displ), nullptr));
        this->drawClippedBitmap(canvas, 0, 400, paint);
        displ = SkImageSource::Make(fLarge);
        paint.setImageFilter(SkDisplacementMapEffect::Make(
            SkDisplacementMapEffect::kB_ChannelSelectorType,
            SkDisplacementMapEffect::kA_ChannelSelectorType,
            40.0f, std::move(displ), nullptr));
        this->drawClippedBitmap(canvas, 100, 400, paint);
        displ = SkImageSource::Make(fLargeW);
        paint.setImageFilter(SkDisplacementMapEffect::Make(
            SkDisplacementMapEffect::kR_ChannelSelectorType,
            SkDisplacementMapEffect::kB_ChannelSelectorType,
            40.0f, std::move(displ), nullptr));
        this->drawClippedBitmap(canvas, 200, 400, paint);
        displ = SkImageSource::Make(fLargeH);
        paint.setImageFilter(SkDisplacementMapEffect::Make(
            SkDisplacementMapEffect::kG_ChannelSelectorType,
            SkDisplacementMapEffect::kA_ChannelSelectorType,
            40.0f, std::move(displ), nullptr));
        this->drawClippedBitmap(canvas, 300, 400, paint);

        // Test for no given displacement input. In this case, both displacement
        // and color should use the same bitmap, given to SkCanvas::drawBitmap()
        // as an input argument.
        paint.setImageFilter(SkDisplacementMapEffect::Make(
            SkDisplacementMapEffect::kG_ChannelSelectorType,
            SkDisplacementMapEffect::kA_ChannelSelectorType,
            40.0f, nullptr, nullptr));
        this->drawClippedBitmap(canvas, 400, 400, paint);
    }

private:
    SkBitmap fBitmap;
    sk_sp<SkImage> fCheckerboard, fSmall, fLarge, fLargeW, fLargeH;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new DisplacementMapGM;)
}
