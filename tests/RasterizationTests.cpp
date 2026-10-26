/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "tests/Test.h"

#include <initializer_list>

DEF_TEST(path_aaa_walk_convex_vertical_clip, reporter) {
    SkPath path = SkPathBuilder()
                          .moveTo(161564.5f, 20.75f)
                          .lineTo(41564.5f, 40.75f)
                          .quadTo(20784.5f, 170.625f, 4.5f, 300.5f)
                          .lineTo(160004.5f, 40.75f)
                          .close()
                          .detach();
    REPORTER_ASSERT(reporter, path.isConvex());

    // Also test a wide oval clipped on the right (similar to the SVG/CSS clip-path PoCs).
    SkPath oval = SkPath::Oval(SkRect::MakeLTRB(-22120.f, -296.f, 160004.5f, 300.f));
    REPORTER_ASSERT(reporter, oval.isConvex());

    for (bool aa : {false, true}) {
        auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(300, 300));
        SkCanvas* canvas = surface->getCanvas();
        canvas->clear(SK_ColorTRANSPARENT);

        SkPaint paint;
        paint.setAntiAlias(aa);
        paint.setColor(SK_ColorBLACK);
        canvas->drawPath(path, paint);

        SkPixmap pm;
        REPORTER_ASSERT(reporter, surface->peekPixels(&pm));
        // At y = 299, the quadratic edge enters the clip from the right (x ~ 298.75 at y = 298.75
        // down to x ~ 124.2 at y = 300), so pixels near the bottom-right corner must be covered.
        REPORTER_ASSERT(reporter, pm.getColor(295, 299) == SK_ColorBLACK);
        // Pixels outside the path (top-left) must remain untouched.
        REPORTER_ASSERT(reporter, pm.getColor(0, 0) == SK_ColorTRANSPARENT);

        canvas->clear(SK_ColorTRANSPARENT);
        canvas->drawPath(oval, paint);
    }
}
