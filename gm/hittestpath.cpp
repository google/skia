/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "src/base/SkRandom.h"

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

DEF_SIMPLE_GM_CAN_FAIL(hittestpath, canvas, errorMsg, 700, 460) {
    if (canvas->recordingContext() || canvas->recorder()) {
        // GPU rasterization results vary greatly from platform to platform. We can't use them as
        // an expected result for our internal SkPath::contains().
        *errorMsg = "This test is for CPU configs only.";
        return skiagm::DrawResult::kSkip;
    }

    SkPathBuilder b;
    SkRandom rand;

    int scale = 300;
    for (int i = 0; i < 4; ++i) {
        // get the random values deterministically
        SkScalar randoms[12];
        for (int index = 0; index < (int) std::size(randoms); ++index) {
            randoms[index] = rand.nextUScalar1();
        }
        b.lineTo(randoms[0] * scale, randoms[1] * scale)
         .quadTo(randoms[2] * scale, randoms[3] * scale,
                 randoms[4] * scale, randoms[5] * scale)
         .cubicTo(randoms[6] * scale, randoms[7] * scale,
                  randoms[8] * scale, randoms[9] * scale,
                  randoms[10] * scale, randoms[11] * scale);
    }

    b.setFillType(SkPathFillType::kEvenOdd);
    b.offset(SkIntToScalar(20), SkIntToScalar(20));

    SkPath path = b.detach();

    test_hittest(canvas, path);

    canvas->translate(SkIntToScalar(scale), 0);
    path.setFillType(SkPathFillType::kWinding);

    test_hittest(canvas, path);
    return skiagm::DrawResult::kOk;
}
