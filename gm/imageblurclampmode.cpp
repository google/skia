/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkImageFilters.h"
#include "tools/ToolUtils.h"

#include <initializer_list>
#include <utility>

static sk_sp<SkImage> make_image(SkCanvas* canvas) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(250, 200);
    auto        surface = ToolUtils::makeSurface(canvas, info);
    SkCanvas* c = surface->getCanvas();
    SkPaint paint;
    paint.setAntiAlias(true);

    paint.setColor(SK_ColorBLUE);
    c->drawRect(SkRect::MakeIWH(info.width(), info.height()), paint);
    paint.setColor(SK_ColorGREEN);
    c->drawCircle(125, 100, 100, paint);
    paint.setColor(SK_ColorRED);
    c->drawRect(SkRect::MakeIWH(80, 80), paint);

    return surface->makeImageSnapshot();
}

static void draw_image(SkCanvas* canvas, const sk_sp<SkImage> image, sk_sp<SkImageFilter> filter) {
    SkAutoCanvasRestore acr(canvas, true);
    SkPaint paint;
    paint.setImageFilter(std::move(filter));

    canvas->translate(SkIntToScalar(30), 0);
    canvas->clipIRect(image->bounds());
    canvas->drawImage(image, 0, 0, SkSamplingOptions(), &paint);
}

namespace skiagm {

// This GM draws one rectangle, one green inscribed circle, and one red square
// with different blur settings.
class ImageBlurClampModeGM : public GM {
public:
    ImageBlurClampModeGM() {
        this->setBGColor(0xFFCCCCCC);
    }

protected:

    SkString onShortName() override {
        return SkString("imageblurclampmode");
    }

    SkISize onISize() override {
        return SkISize::Make(850, 920);
    }

    bool runAsBench() const override { return true; }

    void onDraw(SkCanvas* canvas) override {
        sk_sp<SkImage> image(make_image(canvas));
        sk_sp<SkImageFilter> filter;

        canvas->translate(0, 30);
        // Test different kernel size, including the one to launch 2d Gaussian
        // blur.
        for (auto sigma: { 0.6f, 3.0f, 8.0f, 20.0f }) {
            canvas->save();

            // x-only blur
            filter =  SkImageFilters::Blur(sigma, 0.0f, SkTileMode::kClamp, nullptr);
            draw_image(canvas, image, std::move(filter));
            canvas->translate(image->width() + 20, 0);

            // y-only blur
            filter = SkImageFilters::Blur(0.0f, sigma, SkTileMode::kClamp, nullptr);
            draw_image(canvas, image, std::move(filter));
            canvas->translate(image->width() + 20, 0);

            // both directions
            filter = SkImageFilters::Blur(sigma, sigma, SkTileMode::kClamp, nullptr);
            draw_image(canvas, image, std::move(filter));
            canvas->translate(image->width() + 20, 0);

            canvas->restore();

            canvas->translate(0, image->height() + 20);
        }
    }

private:
    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ImageBlurClampModeGM;)
}  // namespace skiagm
