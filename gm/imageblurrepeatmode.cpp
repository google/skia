/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
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

static sk_sp<SkImage> make_image(SkCanvas* canvas, int direction) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(250, 200);
    auto        surface = ToolUtils::makeSurface(canvas, info);
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
                paint.setAlphaf(0.5f);
            }
            c->drawRect(SkRect::MakeXYWH(x, 0, width, info.height()), paint);
        }
    }

    if (yDirection) {
        for (int y = 0; y < info.height(); y += width) {
            paint.setColor(colors[y/width % 5]);
            if (xDirection) {
                paint.setAlphaf(0.5f);
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
    canvas->clipIRect(image->bounds());
    canvas->drawImage(image, 0, 0, SkSamplingOptions(), &paint);
}

namespace skiagm {

// This GM draws a colorful grids with different blur settings.
class ImageBlurRepeatModeGM : public GM {
public:
    ImageBlurRepeatModeGM() {
        this->setBGColor(0xFFCCCCCC);
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
                  SkImageFilters::Blur(sigma, 0.0f, SkTileMode::kRepeat, nullptr));
            draw_image(canvas, image[0], std::move(filter));
            canvas->translate(image[0]->width() + 20, 0);

            filter = SkImageFilters::Blur(0.0f, sigma, SkTileMode::kRepeat, nullptr);
            draw_image(canvas, image[1], std::move(filter));
            canvas->translate(image[1]->width() + 20, 0);

            filter = SkImageFilters::Blur(sigma, sigma, SkTileMode::kRepeat, nullptr);
            draw_image(canvas, image[2], std::move(filter));
            canvas->translate(image[2]->width() + 20, 0);

            canvas->restore();
            canvas->translate(0, image[0]->height() + 20);
        }
    }

private:
    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ImageBlurRepeatModeGM;)
}  // namespace skiagm

// See skbug.com/10145 for more context, but if the blur doesn't have its own crop rect and
// the canvas is not clipped, repeat can behave strangely (before fixes, this meant:
//  1. The filtered results became semi-transparent when they should have remained opaque.
//  2. The filtered results clip to 3xSigma, which makes sense for the decal tile mode, but not
//     the others.
//  3. The repeat filter interacts non-intuitively when an expanded clip rect intersects the draw
//     geometry (it repeats across the edges of the intersection instead of repeating across the
//     draw and then clipping)).
DEF_SIMPLE_GM(imageblurrepeatunclipped, canvas, 256, 128) {
    // To show translucency
    auto checkerboard = ToolUtils::create_checkerboard_image(256, 128, SK_ColorLTGRAY,
                                                             SK_ColorGRAY, 8);
    canvas->drawImage(checkerboard, 0, 0);

    // Make an image with one red and one blue band
    SkBitmap bmp;
    bmp.allocN32Pixels(100, 20);
    bmp.eraseArea(SkIRect::MakeWH(100, 10), SK_ColorRED);
    bmp.eraseArea(SkIRect::MakeXYWH(0, 10, 100, 10), SK_ColorBLUE);

    auto img = bmp.asImage();
    auto filter = SkImageFilters::Blur(0, 10, SkTileMode::kRepeat, nullptr);
    SkPaint paint;
    paint.setImageFilter(std::move(filter));

    // Draw the blurred image once
    canvas->translate(0, 50);
    canvas->drawImage(img, 0, 0, SkSamplingOptions(), &paint);

    // Draw the blurred image with a clip positioned such that the draw would be excluded except
    // that the image filter causes it to intersect with the clip. Ideally should look like the
    // left image, but clipped to the debug-black rectangle (Narrator: it does not look like that).
    canvas->translate(110, 0);
    canvas->clipRect(SkRect::MakeXYWH(0, -30, 100, 10));
    canvas->drawImage(img, 0, 0, SkSamplingOptions(), &paint);

    // Visualize the clip
    SkPaint line;
    line.setStyle(SkPaint::kStroke_Style);
    canvas->drawRect(SkRect::MakeXYWH(0, -30, 99, 9), line);
}
