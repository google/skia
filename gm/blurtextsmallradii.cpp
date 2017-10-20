/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <tuple>
#include "gm.h"
#include "SkBlurMaskFilter.h"
#include "SkColor.h"
#include "SkGraphics.h"

extern bool gBlurNew;
extern bool gUseBessel;
extern bool gUseSmallAlgo;

static constexpr int ts = 80;

// GM to check the behavior from chrome bug:745290
DEF_SIMPLE_GM(blurSmallRadii, canvas, 16 * ts, 16 * ts) {
    double sigmas[] = {0.0, 0.25, 0.5, 0.75, 1.0, 1.25, 1.5, 1.75, 1.9999, 2.0, 2.25, 2.5};
    SkPaint paint;

    using mt = std::tuple<bool, bool, bool>;

    canvas->translate(0.5f * ts, 0.5f * ts);

    for (auto b : {mt{false, false, true}, mt{true, false, true}, mt{true, true, true},
        mt{false, false, false}}) {
        std::tie(gBlurNew, gUseBessel, gUseSmallAlgo) = b;

        canvas->save();
        for (auto sigma : sigmas) {
            paint.setMaskFilter(nullptr);
            paint.setColor(SK_ColorBLACK);
            paint.setAntiAlias(true);
            paint.setTextSize(ts);
            if (sigma != 0.0) {
                paint.setMaskFilter(SkBlurMaskFilter::Make(kNormal_SkBlurStyle, sigma));
            }
            canvas->drawString("Guest", 0, ts, paint);

            /*
            paint.setMaskFilter(nullptr);
            paint.setColor(SK_ColorWHITE);
            canvas->drawString("Guest", 0, ts, paint);
             */

            canvas->translate(0, 1.2f * ts);
        }
        canvas->restore();
        canvas->translate(4 * ts, 0);
        SkGraphics::PurgeFontCache();
    }
}
