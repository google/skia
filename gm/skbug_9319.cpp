/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"

// Illustrates a bug where the outer portion of the GPU rect blur was too dark with a small sigma.
DEF_SIMPLE_GM(skbug_9319, canvas, 256, 512) {
    SkPaint p;
    p.setAntiAlias(true);
    p.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 0.5f));

    const auto r = SkRect::MakeXYWH(10, 10, 100, 100);

    {
        SkAutoCanvasRestore acr(canvas, true);
        // Clip out interior so that the outer portion stands out.
        canvas->clipRect(r, SkClipOp::kDifference);
        canvas->drawRect(r, p);
    }

    canvas->translate(0, 120);


    // RRect for comparison.
    const auto rr = SkRRect::MakeRectXY(r, .1f, .1f);
    {
        SkAutoCanvasRestore acr(canvas, true);
        canvas->clipRRect(rr, SkClipOp::kDifference);
        canvas->drawRRect(rr, p);
    }
}
