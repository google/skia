/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "tools/fonts/FontToolUtils.h"

// GM to check the behavior from chrome bug:745290
DEF_SIMPLE_GM(blurSmallRadii, canvas, 100, 100) {
    double sigmas[] = {0.5, 0.75, 1.0, 1.5, 2.5};
    SkPaint paint;
    SkFont font = ToolUtils::DefaultPortableFont();

    for (auto sigma : sigmas) {
        paint.setColor(SK_ColorBLACK);
        paint.setAntiAlias(true);
        paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, sigma));
        canvas->drawString("Guest", 20, 10, font, paint);

        paint.setMaskFilter(nullptr);
        paint.setColor(SK_ColorWHITE);
        canvas->drawString("Guest", 20, 10, font, paint);
        canvas->translate(0, 20);
    }
}
