// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE_ANIMATED(Octopus_Generator_Animated, 256, 256, false, 0, 4) {
void paintOctopus(int x, int y, int size_base, SkColor color, SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(color);
    int radius = 3*size_base;
    canvas->drawCircle(x, y, radius, paint);
    for (int leg = 0; leg < 8; ++leg) {
      canvas->drawCircle(x - radius + (2*radius/7.5*leg),
                         y + radius - pow(abs(4-leg), 2), size_base/2 + 2, paint);
    }
    paint.setColor(SkColorSetRGB(std::min(255u, SkColorGetR(color) + 20),
                                 std::min(255u, SkColorGetG(color) + 20),
                                 std::min(255u, SkColorGetB(color) + 20)));
    canvas->drawCircle(x-size_base, y+size_base, size_base/2, paint);
    canvas->drawCircle(x+size_base, y+size_base, size_base/2, paint);
}

void draw(SkCanvas* canvas) {
    SkRandom rand;

    for (int i = 0; i < 400; ++i) {
      float x = rand.nextRangeScalar(0, 256);
      float y = rand.nextRangeScalar(0, 256);
      float s = rand.nextRangeScalar(6, 12);
      SkColor c = rand.nextU() | SkColorSetARGB(255, 0, 0, 0);
      float radius = rand.nextRangeScalar(0, 40);
      float angle = (rand.nextRangeScalar(0, 1) + frame) * 6.28319;
      x += radius * cos(angle);
      y += radius * sin(angle);
      paintOctopus(x, y, s, c, canvas);
    }
}
}  // END FIDDLE
