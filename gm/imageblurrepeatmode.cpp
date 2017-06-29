/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkSurface.h"
#include "SkBlurImageFilter.h"

static sk_sp<SkSurface> make_surface(SkCanvas* canvas, const SkImageInfo& info) {
    auto surface = canvas->makeSurface(info);
    if (!surface) {
        surface = SkSurface::MakeRaster(info);
    }
    return surface;
}

static sk_sp<SkImage> make_image(SkCanvas* canvas, int direction) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(250, 200);
    auto surface = make_surface(canvas, info);
    SkCanvas* c = surface->getCanvas();
    SkPaint paint;
    paint.setAntiAlias(true);

    const SkColor colors[] = {
        SK_ColorRED, SK_ColorBLUE, SK_ColorGREEN, SK_ColorYELLOW, SK_ColorBLACK
    };

    int width = 25;
    bool xDirection = (direction & 0x1) == 1;
    bool yDirection = (direction & 0x2) == 2;
    if (xDirection) {
        for (int x = 0; x < info.width(); x += width) {
            paint.setColor(colors[x/width % 5]);
            if (yDirection) {
                paint.setAlpha(127);
            }
            c->drawRect(SkRect::MakeXYWH(x, 0, width, info.height()), paint);
        }
    }

    if (yDirection) {
        for (int y = 0; y < info.height(); y += width) {
            paint.setColor(colors[y/width % 5]);
            if (xDirection) {
                paint.setAlpha(127);
            }
            c->drawRect(SkRect::MakeXYWH(0, y, info.width(), width), paint);
        }
    }
    return surface->makeImageSnapshot();
}

static void draw_image(SkCanvas* canvas, const sk_sp<SkImage> image, sk_sp<SkImageFilter> filter) {
    SkAutoCanvasRestore acr(canvas, true);
    SkPaint paint;
    paint.setImageFilter(std::move(filter));

    canvas->translate(SkIntToScalar(30), 0);
    canvas->clipRect(SkRect::MakeIWH(image->width(),image->height()));
    canvas->drawImage(image, 0, 0, &paint);
}

namespace skiagm {

// This GM draws a colorful grids with different blur settings.
class ImageBlurRepeatModeGM : public GM {
public:
    ImageBlurRepeatModeGM() {
        this->setBGColor(sk_tool_utils::color_to_565(0xFFCCCCCC));
    }

protected:

    SkString onShortName() override {
        return SkString("imageblurrepeatmode");
    }

    SkISize onISize() override {
        return SkISize::Make(850, 920);
    }

    bool runAsBench() const override { return true; }

    void onDraw(SkCanvas* canvas) override {
        sk_sp<SkImage> image[] =
                { make_image(canvas, 1), make_image(canvas, 2), make_image(canvas, 3) };

        canvas->translate(0, 30);
        // Test different kernel size, including the one to launch 2d Gaussian
        // blur.
        for (auto sigma: { 0.6f, 3.0f, 8.0f, 20.0f }) {
            canvas->save();
            sk_sp<SkImageFilter> filter(
                  SkBlurImageFilter::Make(sigma, 0.0f, nullptr, nullptr,
                                          SkBlurImageFilter::kRepeat_TileMode));
            draw_image(canvas, image[0], std::move(filter));
            canvas->translate(image[0]->width() + 20, 0);

            filter = SkBlurImageFilter::Make(0.0f, sigma, nullptr, nullptr,
                                             SkBlurImageFilter::kRepeat_TileMode);
            draw_image(canvas, image[1], std::move(filter));
            canvas->translate(image[1]->width() + 20, 0);

            filter = SkBlurImageFilter::Make(sigma, sigma, nullptr, nullptr,
                                             SkBlurImageFilter::kRepeat_TileMode);
            draw_image(canvas, image[2], std::move(filter));
            canvas->translate(image[2]->width() + 20, 0);

            canvas->restore();
            canvas->translate(0, image[0]->height() + 20);
        }
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ImageBlurRepeatModeGM;)
}
