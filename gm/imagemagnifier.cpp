/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPixelRef.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypeface.h"
#include "include/effects/SkImageFilters.h"
#include "include/utils/SkRandom.h"
#include "tools/ToolUtils.h"

#include <utility>

#define WIDTH 500
#define HEIGHT 500

DEF_SIMPLE_GM_BG(imagemagnifier, canvas, WIDTH, HEIGHT, SK_ColorBLACK) {
        SkPaint filterPaint;
        filterPaint.setImageFilter(
            SkImageFilters::Magnifier(
                SkRect::MakeXYWH(SkIntToScalar(100), SkIntToScalar(100),
                                 SkIntToScalar(WIDTH / 2),
                                 SkIntToScalar(HEIGHT / 2)),
                100, nullptr));
        canvas->saveLayer(nullptr, &filterPaint);
        const char* str = "The quick brown fox jumped over the lazy dog.";
        SkRandom rand;
        SkFont      font(ToolUtils::create_portable_typeface());
        for (int i = 0; i < 25; ++i) {
            int x = rand.nextULessThan(WIDTH);
            int y = rand.nextULessThan(HEIGHT);
            SkPaint paint;
            paint.setColor(ToolUtils::color_to_565(rand.nextBits(24) | 0xFF000000));
            font.setSize(rand.nextRangeScalar(0, 300));
            canvas->drawString(str, SkIntToScalar(x), SkIntToScalar(y), font, paint);
        }
        canvas->restore();
}

////////////////////////////////////////////////////////////////////////////////
#define WIDTH_HEIGHT 256

static sk_sp<SkImage> make_img() {
    SkBitmap bitmap;
    bitmap.allocN32Pixels(WIDTH_HEIGHT, WIDTH_HEIGHT);
    SkCanvas canvas(bitmap);

    canvas.clear(0x0);

    SkPaint paint;
    paint.setColor(SK_ColorBLUE);

    for (float pos = 0; pos < WIDTH_HEIGHT; pos += 16) {
        canvas.drawLine(0, pos, SkIntToScalar(WIDTH_HEIGHT), pos, paint);
        canvas.drawLine(pos, 0, pos, SkIntToScalar(WIDTH_HEIGHT), paint);
    }

    SkBitmap result;
    result.setInfo(SkImageInfo::MakeS32(WIDTH_HEIGHT, WIDTH_HEIGHT, kPremul_SkAlphaType));
    result.setPixelRef(sk_ref_sp(bitmap.pixelRef()), 0, 0);

    return result.asImage();
}

DEF_SIMPLE_GM_BG(imagemagnifier_cropped, canvas, WIDTH_HEIGHT, WIDTH_HEIGHT, SK_ColorBLACK) {

    sk_sp<SkImage> image(make_img());

    sk_sp<SkImageFilter> imageSource(SkImageFilters::Image(std::move(image)));

    SkRect srcRect = SkRect::MakeWH(SkIntToScalar(WIDTH_HEIGHT-32),
                                    SkIntToScalar(WIDTH_HEIGHT-32));
    srcRect.inset(64.0f, 64.0f);

    constexpr SkScalar kInset = 64.0f;

    // Crop out a 16 pixel ring around the result
    const SkIRect cropRect = SkIRect::MakeXYWH(16, 16, WIDTH_HEIGHT-32, WIDTH_HEIGHT-32);

    SkPaint filterPaint;
    filterPaint.setImageFilter(SkImageFilters::Magnifier(
            srcRect, kInset, std::move(imageSource),  &cropRect));

    canvas->saveLayer(nullptr, &filterPaint);
    canvas->restore();
}
