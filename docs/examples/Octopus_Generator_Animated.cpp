// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"

REG_FIDDLE_ANIMATED(Octopus_Generator_Animated, 256, 256, false, 0, 4) {
#include <random>

void paintOctopus(float x, float y, int size_base, SkColor color, SkCanvas* canvas) {
    // Set up the paint to draw the head and legs
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(color);
    int radius = 3*size_base;
    // Draw the big head
    canvas->drawCircle(x, y, radius, paint);
    // Draw 8 small legs
    for (int leg = 0; leg < 8; ++leg) {
      canvas->drawCircle(x - radius + (2*radius/7.5*leg),
                         y + radius - pow(abs(4-leg), 2), size_base/2 + 2, paint);
    }
    // Make the color a bit lighter for the eyes
    paint.setColor(SkColorSetRGB(std::min(255u, SkColorGetR(color) + 20),
                                 std::min(255u, SkColorGetG(color) + 20),
                                 std::min(255u, SkColorGetB(color) + 20)));
    // Draw the left and right eye
    canvas->drawCircle(x-size_base, y+size_base, size_base/2, paint);
    canvas->drawCircle(x+size_base, y+size_base, size_base/2, paint);
}

void draw(SkCanvas* canvas) {
    // This is called many times, once per frame. To keep the octopods
    // a consistent color, we depend on the RNG using the *same* seed
    // for each of calls (i.e. don't reseed it based on time or something).
    std::default_random_engine rng;
    const auto randScalar = [&rng](float min, float max) -> float {
        return std::uniform_real_distribution<float>(min, max)(rng);
    };
    const auto randInt = [&rng](int min, int max) -> int {
        return std::uniform_int_distribution<int>(min, max)(rng);
    };
    const auto randOpaqueColor = [&rng]() -> SkColor {
        return std::uniform_int_distribution<uint32_t>(0, 0xFFFFFF)(rng) | 0xFF000000;
    };

    // Don't make this too big or rendering will time out.
    constexpr int numOctopods = 20;
    for (int i = 0; i < numOctopods; ++i) {
      float x = randScalar(0, 256);
      float y = randScalar(0, 256);
      int s = randInt(6, 12);
      SkColor c = randOpaqueColor();
      // To make the octopods "swim" we have them move around a circle
      // of a random radius
      float radius = randScalar(0, 40);
      // frame is in range [0, 1.0], so this starts at some fraction of 2pi
      // (e.g. 1/2 pi) and goes until that fraction + 2pi (e.g. 5/2 pi)
      float angle = (randScalar(0, 1) + frame) * 6.28319;
      x += radius * cos(angle);
      y += radius * sin(angle);
      paintOctopus(x, y, s, c, canvas);
    }
}
}  // END FIDDLE
