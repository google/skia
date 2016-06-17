/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "SkDashPathEffect.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkRRect.h"

namespace skiagm {

class ContourStartGM : public GM {
public:
    ContourStartGM() {
        const SkScalar kMaxDashLen = 100;
        const SkScalar kDashGrowth = 1.2f;

        SkSTArray<100, SkScalar> intervals;
        for (SkScalar len = 1; len < kMaxDashLen; len *= kDashGrowth) {
            intervals.push_back(len);
            intervals.push_back(len);
        }

        fDashPaint.setAntiAlias(true);
        fDashPaint.setStyle(SkPaint::kStroke_Style);
        fDashPaint.setStrokeWidth(6);
        fDashPaint.setColor(0xff008000);
        fDashPaint.setPathEffect(SkDashPathEffect::Make(intervals.begin(), intervals.count(), 0));

        fPointsPaint.setColor(0xff800000);
        fPointsPaint.setStrokeWidth(3);

        fRect = SkRect::MakeLTRB(10, 10, 100, 70);
    }

protected:
    SkString onShortName() override {
        return SkString("contour_start");
    }

    SkISize onISize() override { return SkISize::Make(kImageWidth, kImageHeight); }

    void onDraw(SkCanvas* canvas) override {

        drawDirs(canvas, [](const SkRect& rect, SkPath::Direction dir, unsigned startIndex) {
            SkPath path;
            path.addRect(rect, dir, startIndex);
            return path;
        });

        drawDirs(canvas, [](const SkRect& rect, SkPath::Direction dir, unsigned startIndex) {
            SkPath path;
            path.addOval(rect, dir, startIndex);
            return path;
        });

        drawDirs(canvas, [](const SkRect& rect, SkPath::Direction dir, unsigned startIndex) {
            SkRRect rrect;
            const SkVector radii[4] = { {15, 15}, {15, 15}, {15, 15}, {15, 15}};
            rrect.setRectRadii(rect, radii);

            SkPath path;
            path.addRRect(rrect, dir, startIndex);
            return path;
        });

        drawDirs(canvas, [](const SkRect& rect, SkPath::Direction dir, unsigned startIndex) {
            SkRRect rrect;
            rrect.setRect(rect);

            SkPath path;
            path.addRRect(rrect, dir, startIndex);
            return path;
        });

        drawDirs(canvas, [](const SkRect& rect, SkPath::Direction dir, unsigned startIndex) {
            SkRRect rrect;
            rrect.setOval(rect);

            SkPath path;
            path.addRRect(rrect, dir, startIndex);
            return path;
        });

    }

private:
    static const int kImageWidth = 1200;
    static const int kImageHeight = 600;

    SkPaint fDashPaint, fPointsPaint;
    SkRect  fRect;

    void drawDirs(SkCanvas* canvas,
                  SkPath (*makePath)(const SkRect&, SkPath::Direction, unsigned)) const {
        drawOneColumn(canvas, SkPath::kCW_Direction, makePath);
        canvas->translate(kImageWidth / 10, 0);
        drawOneColumn(canvas, SkPath::kCCW_Direction, makePath);
        canvas->translate(kImageWidth / 10, 0);
    }

    void drawOneColumn(SkCanvas* canvas, SkPath::Direction dir,
                       SkPath (*makePath)(const SkRect&, SkPath::Direction, unsigned)) const {
        SkAutoCanvasRestore acr(canvas, true);

        for (unsigned i = 0; i < 8; ++i) {
            const SkPath path = makePath(fRect, dir, i);
            canvas->drawPath(path, fDashPaint);

            const int n = path.countPoints();
            SkAutoTArray<SkPoint> points(n);
            path.getPoints(points.get(), n);
            canvas->drawPoints(SkCanvas::kPoints_PointMode, n, points.get(), fPointsPaint);

            canvas->translate(0, kImageHeight / 8);
        }
    }

    typedef GM INHERITED;
};

DEF_GM( return new ContourStartGM(); )

} // namespace skiagm
