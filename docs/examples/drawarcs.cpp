// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"

#include <random>

REG_FIDDLE(drawarcs, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(8);

    std::default_random_engine rng;
    const auto randScalar = [&rng](SkScalar min, SkScalar max) -> SkScalar {
        return std::uniform_real_distribution<SkScalar>(min, max)(rng);
    };
    const auto randOpaqueColor = [&rng]() -> SkColor {
        return std::uniform_int_distribution<uint32_t>(0, 0xFFFFFF)(rng) | 0xFF000000;
    };
    SkPath path;

    for (int i = 0; i < 100; ++i) {
        SkScalar x = randScalar(0, 200);
        SkScalar y = randScalar(0, 200);

        path.rewind();
        path.addArc(SkRect::MakeXYWH(x, y, 70, 70), randScalar(0, 360),
                    randScalar(0, 360));
        paint.setColor(randOpaqueColor());
        canvas->drawPath(path, paint);
    }
}
}  // END FIDDLE
