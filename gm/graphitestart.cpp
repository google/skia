/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"

namespace {

SkPaint create_image_shader_paint() {
    SkImageInfo ii = SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    SkBitmap bitmap;

    bitmap.allocPixels(ii);
    bitmap.eraseColor(SK_ColorWHITE);

    SkCanvas canvas(bitmap);

    SkPaint redPaint;
    redPaint.setColor(SK_ColorRED);
    canvas.drawCircle(50, 50, 50, redPaint);

    sk_sp<SkImage> img = SkImage::MakeFromBitmap(bitmap);

    SkPaint p;
    p.setShader(img->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat, SkSamplingOptions()));
    return p;
}

} // anonymous namespace

namespace skiagm {

// This is just for bootstrapping Graphite.
class GraphiteStartGM : public GM {
public:
    GraphiteStartGM() {
        this->setBGColor(0xFFCCCCCC);
    }

protected:
    SkString onShortName() override {
        return SkString("graphitestart");
    }

    SkISize onISize() override {
        return SkISize::Make(256, 256);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint p1, p2;

        p1.setColor(SK_ColorRED);
        p2.setColor(SK_ColorGREEN);

        canvas->drawRect({  2,   2, 127, 127}, p1);
        canvas->drawRect({129, 129, 255, 255}, p2);
        canvas->drawRect({  2, 129, 127, 255}, create_image_shader_paint());
    }
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new GraphiteStartGM;)

}  // namespace skiagm
