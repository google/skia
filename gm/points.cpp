/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "src/base/SkRandom.h"

#include <stddef.h>

namespace skiagm {

class PointsGM : public GM {
public:
    PointsGM() {}

protected:
    SkString getName() const override { return SkString("points"); }

    SkISize getISize() override { return SkISize::Make(640, 490); }

    static void fill_pts(SkPoint pts[], size_t n, SkRandom* rand) {
        for (size_t i = 0; i < n; i++) {
            // Compute these independently and store in variables, rather
            // than in the parameter-passing expression, to get consistent
            // evaluation order across compilers.
            SkScalar y = rand->nextUScalar1() * 480;
            SkScalar x = rand->nextUScalar1() * 640;
            pts[i].set(x, y);
        }
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->translate(SK_Scalar1, SK_Scalar1);

        SkRandom rand;
        SkPaint  p0, p1, p2, p3;
        const size_t n = 99;

        p0.setColor(SK_ColorRED);
        p1.setColor(SK_ColorGREEN);
        p2.setColor(SK_ColorBLUE);
        p3.setColor(SK_ColorWHITE);

        p0.setStrokeWidth(SkIntToScalar(4));
        p2.setStrokeCap(SkPaint::kRound_Cap);
        p2.setStrokeWidth(SkIntToScalar(6));

        SkPoint* pts = new SkPoint[n];
        fill_pts(pts, n, &rand);

        canvas->drawPoints(SkCanvas::kPolygon_PointMode, n, pts, p0);
        canvas->drawPoints(SkCanvas::kLines_PointMode, n, pts, p1);
        canvas->drawPoints(SkCanvas::kPoints_PointMode, n, pts, p2);
        canvas->drawPoints(SkCanvas::kPoints_PointMode, n, pts, p3);

        delete[] pts;
    }

private:
    using INHERITED = GM;
};
DEF_GM( return new PointsGM; )
}  // namespace skiagm

#include "include/core/SkMaskFilter.h"

DEF_SIMPLE_GM(points_maskfilter, canvas, 512, 256) {
    constexpr int N = 30;
    SkPoint pts[N];

    SkRandom rand;
    for (SkPoint& p : pts) {
        p.fX = rand.nextF() * 220 + 18;
        p.fY = rand.nextF() * 220 + 18;
    }

    auto mf = SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 6);
    const SkPaint::Cap caps[] = { SkPaint::kSquare_Cap, SkPaint::kRound_Cap };

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStroke(true);
    paint.setStrokeWidth(10);

    for (auto cap : caps) {
        paint.setStrokeCap(cap);

        paint.setMaskFilter(mf);
        paint.setColor(SK_ColorBLACK);
        canvas->drawPoints(SkCanvas::kPoints_PointMode, N, pts, paint);

        paint.setMaskFilter(nullptr);
        paint.setColor(SK_ColorRED);
        canvas->drawPoints(SkCanvas::kPoints_PointMode, N, pts, paint);

        canvas->translate(256, 0);
    }
}
