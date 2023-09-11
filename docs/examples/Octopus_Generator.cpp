// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"

REG_FIDDLE(Octopus_Generator, 256, 256, false, 0) {
#include <random>

void paintOctopus(int x, int y, int size_base, SkColor color, SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(color);
    int radius = 3 * size_base;
    canvas->drawCircle(x, y, radius, paint);
    for (int leg = 0; leg < 8; ++leg) {
        canvas->drawCircle(x - radius + (2 * radius / 7.5 * leg),
                           y + radius - pow(abs(4 - leg), 2), size_base / 2 + 2, paint);
    }
    paint.setColor(SkColorSetRGB(std::min(255u, SkColorGetR(color) + 20),
                                 std::min(255u, SkColorGetG(color) + 20),
                                 std::min(255u, SkColorGetB(color) + 20)));
    canvas->drawCircle(x - size_base, y + size_base, size_base / 2, paint);
    canvas->drawCircle(x + size_base, y + size_base, size_base / 2, paint);
}

void draw(SkCanvas* canvas) {
    std::default_random_engine rng;
    const auto randScalar = [&rng](SkScalar min, SkScalar max) -> SkScalar {
        return std::uniform_real_distribution<SkScalar>(min, max)(rng);
    };
    const auto randOpaqueColor = [&rng]() -> SkColor {
        return std::uniform_int_distribution<uint32_t>(0, 0xFFFFFF)(rng) | 0xFF000000;
    };

    for (int i = 0; i < 400; ++i) {
        paintOctopus(randScalar(0, 256), randScalar(0, 256),
                     randScalar(6, 12), randOpaqueColor(), canvas);
    }
}
}  // END FIDDLE
