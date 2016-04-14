/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkImageSource.h"
#include "SkMagnifierImageFilter.h"
#include "SkRandom.h"
#include "SkSurface.h"

#define WIDTH 500
#define HEIGHT 500

DEF_SIMPLE_GM_BG(imagemagnifier, canvas, WIDTH, HEIGHT, SK_ColorBLACK) {
        SkPaint filterPaint;
        filterPaint.setImageFilter(
            SkMagnifierImageFilter::Make(
                SkRect::MakeXYWH(SkIntToScalar(100), SkIntToScalar(100),
                                 SkIntToScalar(WIDTH / 2),
                                 SkIntToScalar(HEIGHT / 2)),
                100, nullptr));
        canvas->saveLayer(nullptr, &filterPaint);
        const char* str = "The quick brown fox jumped over the lazy dog.";
        SkRandom rand;
        for (int i = 0; i < 25; ++i) {
            int x = rand.nextULessThan(WIDTH);
            int y = rand.nextULessThan(HEIGHT);
            SkPaint paint;
            sk_tool_utils::set_portable_typeface(&paint);
            paint.setColor(sk_tool_utils::color_to_565(rand.nextBits(24) | 0xFF000000));
            paint.setTextSize(rand.nextRangeScalar(0, 300));
            paint.setAntiAlias(true);
            canvas->drawText(str, strlen(str), SkIntToScalar(x),
                             SkIntToScalar(y), paint);
        }
        canvas->restore();
}

////////////////////////////////////////////////////////////////////////////////
#define WIDTH_HEIGHT 256

static sk_sp<SkImage> make_img() {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(WIDTH_HEIGHT, WIDTH_HEIGHT);

    sk_sp<SkSurface> surf(SkSurface::MakeRaster(info));

    SkCanvas* canvas = surf->getCanvas();

    canvas->clear(0x0);

    SkPaint paint;
    paint.setColor(SK_ColorBLUE);

    for (float pos = 0; pos < WIDTH_HEIGHT; pos += 16) {
        canvas->drawLine(0, pos, SkIntToScalar(WIDTH_HEIGHT), pos, paint);
        canvas->drawLine(pos, 0, pos, SkIntToScalar(WIDTH_HEIGHT), paint);
    }

    return surf->makeImageSnapshot();
}

DEF_SIMPLE_GM_BG(imagemagnifier_cropped, canvas, WIDTH_HEIGHT, WIDTH_HEIGHT, SK_ColorBLACK) {

    sk_sp<SkImage> image(make_img());

    sk_sp<SkImageFilter> imageSource(SkImageSource::Make(std::move(image)));

    SkRect srcRect = SkRect::MakeWH(SkIntToScalar(WIDTH_HEIGHT-32),
                                    SkIntToScalar(WIDTH_HEIGHT-32));
    srcRect.inset(64.0f, 64.0f);

    static const SkScalar kInset = 64.0f;

    // Crop out a 16 pixel ring around the result
    const SkRect rect = SkRect::MakeXYWH(16, 16, WIDTH_HEIGHT-32, WIDTH_HEIGHT-32);
    SkImageFilter::CropRect cropRect(rect);

    SkPaint filterPaint;
    filterPaint.setImageFilter(SkMagnifierImageFilter::Make(srcRect, kInset,
                                                            std::move(imageSource),
                                                            &cropRect));

    canvas->saveLayer(nullptr, &filterPaint);
    canvas->restore();
}
