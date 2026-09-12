// Copyright 2019 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"

#include <array>

REG_FIDDLE(Path_lineTo_2, 256, 100, false, 0) {
void draw(SkCanvas* canvas) {
    SkPathBuilder path;
    static constexpr auto oxo = std::to_array<SkVector>({
        SkVector{25, 25}, SkVector{35, 35}, SkVector{25, 35}, SkVector{35, 25},
        SkVector{40, 20}, SkVector{40, 80}, SkVector{60, 20}, SkVector{60, 80},
        SkVector{20, 40}, SkVector{80, 40}, SkVector{20, 60}, SkVector{80, 60}
    });
    for (unsigned i = 0; i < std::size(oxo); i += 2) {
        path.moveTo(oxo[i]);
        path.lineTo(oxo[i + 1]);
    }
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawPath(path.detach(), paint);
}
}  // END FIDDLE
