/*
* Copyright 2017 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "gm.h"
#include "SkSurface.h"

// This GM reproduces skia:6783, which demonstrated a bug in repeat and mirror
// image sampling tiling modes as implemented in software.  We want to tile to
// [0,limit), and the old incorrect logic was:
//
//    limit = ulp_before(limit)
//    val = val - floor(val/limit)*limit    (This is repeat; mirror is similar.)
//
// while the correct logic is more like:
//
//    val = val - floor(val/limit)*limit
//    val = min(val, ulp_before(limit))
//
// You would see ugly jaggies on the blue/yellow edge near the bottom left if
// the bug were still present.  All stripes should now look roughly the same.

DEF_SIMPLE_GM(bug6783, canvas, 500, 500) {
    sk_sp<SkSurface> surface = SkSurface::MakeRasterN32Premul(100, 100);

    SkPaint p;
    p.setColor(SK_ColorYELLOW);
    surface->getCanvas()->drawPaint(p);
    p.setColor(SK_ColorBLUE);
    surface->getCanvas()->drawRect(SkRect::MakeWH(50, 100), p);

    sk_sp<SkImage> img = surface->makeImageSnapshot();

    SkMatrix m = SkMatrix::Concat(SkMatrix::MakeTrans(25, 214),
                                  SkMatrix::MakeScale(2, 2));
    m.preSkew(0.5f, 0.5f);

    // The bug was present at all filter levels, but you might not notice it at kNone.
    p.setFilterQuality(kLow_SkFilterQuality);

    // It's only important to repeat or mirror in x to show off the bug.
    p.setShader(img->makeShader(SkTileMode::kRepeat, SkTileMode::kClamp, &m));
    canvas->drawPaint(p);
}
