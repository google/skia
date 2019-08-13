/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "skia.h"
#include <chrono>

static void draw(SkCanvas* canvas);
DEF_SIMPLE_GM(fiddle, canvas, 256, 256) { draw(canvas); }

void draw(SkCanvas* canvas) {
    const int w = canvas->imageInfo().width(),
              h = canvas->imageInfo().height();

    sk_sp<SkImage> image;
    {
        SkBitmap bitmap;
        bitmap.allocPixels(canvas->imageInfo().makeAlphaType(kOpaque_SkAlphaType));

        auto pixels = (uint32_t*)bitmap.getPixels();
        for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++) {
            *pixels++ = y % 8 ? 0xff0000ff
                              : 0xff00ffff;
        }
        image = SkImage::MakeFromBitmap(bitmap);
    }

    SkMatrix matrix;
    matrix.setRotate(45);

    SkPaint paint;
    paint.setFilterQuality(kLow_SkFilterQuality);
    paint.setShader(image->makeShader(SkTileMode::kMirror,
                                      SkTileMode::kClamp,
                                      &matrix));

    using clock = std::chrono::steady_clock;

    auto start = clock::now();
    canvas->drawRect({0,0, (float)w,(float)h}, paint);
    auto stop = clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count();
    printf("\n%llu µs\n", elapsed);
}
