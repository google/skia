// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"

REG_FIDDLE(Octopus_Generator, 256, 256, false, 0) {
#include <random>

void paintOctopus(float x, float y, int size_base, SkColor color, SkCanvas* canvas) {
    // Set up the paint to draw the head and legs
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(color);
    int radius = 3 * size_base;
    // Draw the big head
    canvas->drawCircle(x, y, radius, paint);
    // Draw 8 small legs
    for (int leg = 0; leg < 8; ++leg) {
        canvas->drawCircle(x - radius + (2 * radius / 7.5 * leg),
                           y + radius - pow(abs(4 - leg), 2), size_base / 2 + 2, paint);
    }
    // Make the color a bit lighter for the eyes
    paint.setColor(SkColorSetRGB(std::min(255u, SkColorGetR(color) + 20),
                                 std::min(255u, SkColorGetG(color) + 20),
                                 std::min(255u, SkColorGetB(color) + 20)));
    // Draw the left and right eye
    canvas->drawCircle(x - size_base, y + size_base, size_base / 2, paint);
    canvas->drawCircle(x + size_base, y + size_base, size_base / 2, paint);
}

void draw(SkCanvas* canvas) {
    // Some helper functions to create random coordinates, sizes, and colors
    std::default_random_engine rng;
    const auto randScalar = [&rng](SkScalar min, SkScalar max) -> SkScalar {
        return std::uniform_real_distribution<SkScalar>(min, max)(rng);
    };
    const auto randOpaqueColor = [&rng]() -> SkColor {
        return std::uniform_int_distribution<uint32_t>(0, 0xFFFFFF)(rng) | 0xFF000000;
    };

    constexpr int numOctopods = 400;
    for (int i = 0; i < numOctopods; ++i) {
        paintOctopus(randScalar(0, 256), randScalar(0, 256),
                     randScalar(6, 12), randOpaqueColor(), canvas);
    }
}
}  // END FIDDLE
