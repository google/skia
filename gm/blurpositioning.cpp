/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/effects/SkBlurImageFilter.h"

// For all sigma, the black box should be centered in the red outline. For small sigma,
// the rectangle is not blurred, but still must be centered properly.

DEF_SIMPLE_GM(check_small_sigma_offset, canvas, 200, 1200) {

    for (auto sigma : {0.0, 0.1, 0.2, 0.3, 0.4, 0.6, 0.8, 1.0, 1.2}) {
        // border calculation from SkBlurImageFilter
        int border = SkScalarCeilToInt(sigma * 3);

        SkRect r = SkRect::MakeXYWH(50, 50, 100, 50);
        SkRect b = r.makeOutset(border + 1, border + 1);
        b.inset(0.5f, 0.5f);
        SkPaint p;
        p.setColor(SK_ColorRED);
        p.setStyle(SkPaint::Style::kStroke_Style);

        canvas->drawRect(b, p);

        p.reset();
        p.setColor(SK_ColorBLACK);
        p.setImageFilter(SkBlurImageFilter::Make(sigma, sigma, nullptr));
        canvas->drawRect(r, p);

        canvas->translate(0, 100);
    }
}
