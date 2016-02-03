/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkPath.h"
#include "SkRandom.h"

static void test_hittest(SkCanvas* canvas, const SkPath& path) {
    SkPaint paint;
    SkRect r = path.getBounds();

    paint.setColor(SK_ColorRED);
    canvas->drawPath(path, paint);

    const SkScalar MARGIN = SkIntToScalar(4);

    paint.setColor(0x800000FF);
    for (SkScalar y = r.fTop + SK_ScalarHalf - MARGIN; y < r.fBottom + MARGIN; y += SK_Scalar1) {
        for (SkScalar x = r.fLeft + SK_ScalarHalf - MARGIN; x < r.fRight + MARGIN; x += SK_Scalar1) {
            if (path.contains(x, y)) {
                canvas->drawPoint(x, y, paint);
            }
        }
    }
}

DEF_SIMPLE_GM(hittestpath, canvas, 700, 460) {
        SkPath path;
        SkRandom rand;

        int scale = 300;
        for (int i = 0; i < 4; ++i) {
            // get the random values deterministically
            SkScalar randoms[12];
            for (int index = 0; index < (int) SK_ARRAY_COUNT(randoms); ++index) {
                randoms[index] = rand.nextUScalar1();
            }
            path.lineTo(randoms[0] * scale, randoms[1] * scale);
            path.quadTo(randoms[2] * scale, randoms[3] * scale,
                        randoms[4] * scale, randoms[5] * scale);
            path.cubicTo(randoms[6] * scale, randoms[7] * scale,
                         randoms[8] * scale, randoms[9] * scale,
                         randoms[10] * scale, randoms[11] * scale);
        }

        path.setFillType(SkPath::kEvenOdd_FillType);
        path.offset(SkIntToScalar(20), SkIntToScalar(20));

        test_hittest(canvas, path);

        canvas->translate(SkIntToScalar(scale), 0);
        path.setFillType(SkPath::kWinding_FillType);

        test_hittest(canvas, path);
}
