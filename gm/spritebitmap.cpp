/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/effects/SkImageFilters.h"
#include "tools/ToolUtils.h"

#include <utility>

static void make_bm(SkBitmap* bm) {
    bm->allocN32Pixels(100, 100);
    bm->eraseColor(SK_ColorBLUE);

    SkCanvas canvas(*bm);
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorRED);
    canvas.drawCircle(50, 50, 50, paint);
}

static void draw_1_bitmap(SkCanvas* canvas, const SkBitmap& bm, bool doClip,
                         int dx, int dy, sk_sp<SkImageFilter> filter) {
    SkAutoCanvasRestore acr(canvas, true);
    SkPaint paint;

    SkRect clipR = SkRect::MakeXYWH(SkIntToScalar(dx),
                                    SkIntToScalar(dy),
                                    SkIntToScalar(bm.width()),
                                    SkIntToScalar(bm.height()));

    paint.setImageFilter(std::move(filter));
    clipR.inset(5, 5);

    canvas->translate(SkIntToScalar(bm.width() + 20), 0);

    if (doClip) {
        canvas->save();
        canvas->clipRect(clipR);
    }
    canvas->drawImage(bm.asImage(), SkIntToScalar(dx), SkIntToScalar(dy),
                      SkSamplingOptions(), &paint);
    if (doClip) {
        canvas->restore();
    }
}

/**
 *  Compare output of drawSprite and drawBitmap (esp. clipping and imagefilters)
 */
class SpriteBitmapGM : public skiagm::GM {
public:
    SpriteBitmapGM() {}

protected:
    SkString getName() const override { return SkString("spritebitmap"); }

    SkISize getISize() override { return SkISize::Make(640, 480); }

    void onDraw(SkCanvas* canvas) override {
        SkBitmap bm;
        make_bm(&bm);

        int dx = 10;
        int dy = 10;

        SkScalar sigma = 8;
        sk_sp<SkImageFilter> filter(SkImageFilters::Blur(sigma, sigma, nullptr));

        draw_1_bitmap(canvas, bm, false, dx, dy, nullptr);
        dy += bm.height() + 20;
        draw_1_bitmap(canvas, bm, false, dx, dy, filter);
        dy += bm.height() + 20;
        draw_1_bitmap(canvas, bm, true, dx, dy, nullptr);
        dy += bm.height() + 20;
        draw_1_bitmap(canvas, bm, true, dx, dy, filter);
    }

private:
    using INHERITED = GM;
};
DEF_GM( return new SpriteBitmapGM; )

// b/41322892 : The CPU backend tries to detect when an image draw is landing perfectly on pixel
// centers, so it can use a faster sprite-blitting path. That code just assumes that ANY translation
// can be pixel-snapped. Image-shaders used to behave like this, but were fixed long-ago.
// The correct result here is for ALL rectangles to be averaged to grey, rather than a pixel-snapped
// black and white checkerboard.
//
// This GM now tests that linear filtering is preserved for subpixel translation, across several
// different possible methods of drawing.
DEF_SIMPLE_GM_BG(drawimagerect_filter, canvas, 180, 60, SK_ColorWHITE) {
    auto image = ToolUtils::create_checkerboard_image(50, 50, SK_ColorWHITE, SK_ColorBLACK, 1);
    SkSamplingOptions sampling{SkFilterMode::kLinear};

    canvas->translate(5, 5);
    canvas->drawImage(image, 0.5f, 0.5f, sampling);

    canvas->translate(60, 0);
    canvas->drawImageRect(image, {0.5f, 0.5f, 50.5f, 50.5f}, sampling);

    auto shader = image->makeShader(sampling);
    canvas->translate(60, 0);
    SkPaint paint;
    SkMatrix offset = SkMatrix::Translate(0.5f, 0.5f);
    paint.setShader(image->makeShader(sampling, &offset));
    canvas->drawRect({0.0f, 0.0f, 50.0f, 50.0f}, paint);
}
