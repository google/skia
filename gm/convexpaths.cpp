/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTArray.h"
#include "src/base/SkRandom.h"

using namespace skia_private;

namespace {

class SkDoOnce {
public:
    SkDoOnce() { fDidOnce = false; }
    // Make noncopyable
    SkDoOnce(SkDoOnce&) = delete;
    SkDoOnce& operator=(SkDoOnce&) = delete;

    bool needToDo() const { return !fDidOnce; }
    bool alreadyDone() const { return fDidOnce; }
    void accomplished() {
        SkASSERT(!fDidOnce);
        fDidOnce = true;
    }

private:
    bool fDidOnce;
};

class ConvexPathsGM : public skiagm::GM {
    SkDoOnce fOnce;

    void onOnceBeforeDraw() override { this->setBGColor(0xFF000000); }

    SkString getName() const override { return SkString("convexpaths"); }

    SkISize getISize() override { return {1200, 1100}; }

    void makePaths() {
        if (fOnce.alreadyDone()) {
            return;
        }
        fOnce.accomplished();

        SkPathBuilder b;
        fPaths.push_back(b.moveTo(0, 0)
                          .quadTo(50, 100, 0, 100)
                          .lineTo(0, 0)
                          .detach());

        fPaths.push_back(b.moveTo(0, 50)
                          .quadTo(50, 0, 100, 50)
                          .quadTo(50, 100, 0, 50)
                          .detach());

        fPaths.push_back(SkPath::Rect({0, 0, 100, 100}, SkPathDirection::kCW));
        fPaths.push_back(SkPath::Rect({0, 0, 100, 100}, SkPathDirection::kCCW));
        fPaths.push_back(SkPath::Circle(50, 50, 50, SkPathDirection::kCW));
        fPaths.push_back(SkPath::Oval(SkRect::MakeXYWH(0, 0, 50, 100), SkPathDirection::kCW));
        fPaths.push_back(SkPath::Oval(SkRect::MakeXYWH(0, 0, 100, 5), SkPathDirection::kCCW));
        fPaths.push_back(SkPath::Oval(SkRect::MakeXYWH(0, 0, 1, 100), SkPathDirection::kCCW));
        fPaths.push_back(SkPath::RRect(SkRRect::MakeRectXY({0, 0, 100, 100}, 40, 20),
                                       SkPathDirection::kCW));

        // large number of points
        static constexpr int kLength = 100;
        static constexpr int kPtsPerSide = (1 << 12);

        b.moveTo(0, 0);
        for (int i = 1; i < kPtsPerSide; ++i) { // skip the first point due to moveTo.
            b.lineTo(kLength * SkIntToScalar(i) / kPtsPerSide, 0);
        }
        for (int i = 0; i < kPtsPerSide; ++i) {
            b.lineTo(kLength, kLength * SkIntToScalar(i) / kPtsPerSide);
        }
        for (int i = kPtsPerSide; i > 0; --i) {
            b.lineTo(kLength * SkIntToScalar(i) / kPtsPerSide, kLength);
        }
        for (int i = kPtsPerSide; i > 0; --i) {
            b.lineTo(0, kLength * SkIntToScalar(i) / kPtsPerSide);
        }
        fPaths.push_back(b.detach());

        // shallow diagonals
        fPaths.push_back(SkPath::Polygon({{0,0}, {100,1}, {98,100}, {3,96}}, false));

        fPaths.push_back(b.arcTo(SkRect::MakeXYWH(0, 0, 50, 100), 25, 130, false)
                          .detach());

        // cubics
        fPaths.push_back(b.cubicTo(  1,  1, 10,  90, 0, 100).detach());
        fPaths.push_back(b.cubicTo(100, 50, 20, 100, 0,   0).detach());

        // path that has a cubic with a repeated first control point and
        // a repeated last control point.
        fPaths.push_back(b.moveTo(10, 10)
                          .cubicTo(10, 10, 10, 0, 20, 0)
                          .lineTo(40, 0)
                          .cubicTo(40, 0, 50, 0, 50, 10)
                          .detach());

        // path that has two cubics with repeated middle control points.
        fPaths.push_back(b.moveTo(10, 10)
                          .cubicTo(10, 0, 10, 0, 20, 0)
                          .lineTo(40, 0)
                          .cubicTo(50, 0, 50, 0, 50, 10)
                          .detach());

        // cubic where last three points are almost a line
        fPaths.push_back(b.moveTo(0, 228.0f/8)
                          .cubicTo( 628.0f/ 8,  82.0f/8,
                                   1255.0f/ 8, 141.0f/8,
                                   1883.0f/ 8, 202.0f/8)
                          .detach());

        // flat cubic where the at end point tangents both point outward.
        fPaths.push_back(b.moveTo(10, 0)
                          .cubicTo(0, 1, 30, 1, 20, 0)
                          .detach());

        // flat cubic where initial tangent is in, end tangent out
        fPaths.push_back(b.moveTo(0, 0)
                          .cubicTo(10, 1, 30, 1, 20, 0)
                          .detach());

        // flat cubic where initial tangent is out, end tangent in
        fPaths.push_back(b.moveTo(10, 0)
                          .cubicTo(0, 1, 20, 1, 30, 0)
                          .detach());

        // triangle where one edge is a degenerate quad
        fPaths.push_back(b.moveTo(8.59375f, 45)
                          .quadTo(16.9921875f,   45,
                                  31.25f,        45)
                          .lineTo(100,          100)
                          .lineTo(8.59375f,      45)
                          .detach());

        // triangle where one edge is a quad with a repeated point
        fPaths.push_back(b.moveTo(0, 25)
                          .lineTo(50, 0)
                          .quadTo(50, 50, 50, 50)
                          .detach());

        // triangle where one edge is a cubic with a 2x repeated point
        fPaths.push_back(b.moveTo(0, 25)
                          .lineTo(50, 0)
                          .cubicTo(50, 0, 50, 50, 50, 50)
                          .detach());

        // triangle where one edge is a quad with a nearly repeated point
        fPaths.push_back(b.moveTo(0, 25)
                          .lineTo(50, 0)
                          .quadTo(50, 49.95f, 50, 50)
                          .detach());

        // triangle where one edge is a cubic with a 3x nearly repeated point
        fPaths.push_back(b.moveTo(0, 25)
                          .lineTo(50, 0)
                          .cubicTo(50, 49.95f, 50, 49.97f, 50, 50)
                          .detach());

        // triangle where there is a point degenerate cubic at one corner
        fPaths.push_back(b.moveTo(0, 25)
                          .lineTo(50, 0)
                          .lineTo(50, 50)
                          .cubicTo(50, 50, 50, 50, 50, 50)
                          .detach());

        // point line
        fPaths.push_back(SkPath::Line({50, 50}, {50, 50}));

        // point quad
        fPaths.push_back(b.moveTo(50, 50)
                          .quadTo(50, 50, 50, 50)
                          .detach());

        // point cubic
        fPaths.push_back(b.moveTo(50, 50)
                          .cubicTo(50, 50, 50, 50, 50, 50)
                          .detach());

        // moveTo only paths
        fPaths.push_back(b.moveTo(0, 0)
                          .moveTo(0, 0)
                          .moveTo(1, 1)
                          .moveTo(1, 1)
                          .moveTo(10, 10)
                          .detach());

        fPaths.push_back(b.moveTo(0, 0)
                          .moveTo(0, 0)
                          .detach());

        // line degenerate
        fPaths.push_back(b.lineTo(100, 100).detach());
        fPaths.push_back(b.quadTo(100, 100, 0, 0).detach());
        fPaths.push_back(b.quadTo(100, 100, 50, 50).detach());
        fPaths.push_back(b.quadTo(50, 50, 100, 100).detach());
        fPaths.push_back(b.cubicTo(0, 0, 0, 0, 100, 100).detach());

        // skbug.com/8928
        fPaths.push_back(b.moveTo(16.875f, 192.594f)
                          .cubicTo(45.625f, 192.594f, 74.375f, 192.594f, 103.125f, 192.594f)
                          .cubicTo(88.75f, 167.708f, 74.375f, 142.823f, 60, 117.938f)
                          .cubicTo(45.625f, 142.823f, 31.25f, 167.708f, 16.875f, 192.594f)
                          .close()
                          .detach());
        SkMatrix m;
        m.setAll(0.1f, 0, -1, 0, 0.115207f, -2.64977f, 0, 0, 1);
        fPaths.back().transform(m);

        // small circle. This is listed last so that it has device coords far
        // from the origin (small area relative to x,y values).
        fPaths.push_back(SkPath::Circle(0, 0, 1.2f));
    }

    void onDraw(SkCanvas* canvas) override {
        this->makePaths();

        SkPaint paint;
        paint.setAntiAlias(true);
        SkRandom rand;
        canvas->translate(20, 20);

        // As we've added more paths this has gotten pretty big. Scale the whole thing down.
        canvas->scale(2.0f/3, 2.0f/3);

        for (int i = 0; i < fPaths.size(); ++i) {
            canvas->save();
            // position the path, and make it at off-integer coords.
            canvas->translate(200.0f * (i % 5) + 1.0f/10,
                              200.0f * (i / 5) + 9.0f/10);
            SkColor color = rand.nextU();
            color |= 0xff000000;
            paint.setColor(color);
#if 0       // This hitting on 32bit Linux builds for some paths. Temporarily disabling while it is
            // debugged.
            SkASSERT(fPaths[i].isConvex());
#endif
            canvas->drawPath(fPaths[i], paint);
            canvas->restore();
        }
    }

    TArray<SkPath> fPaths;
};
}  // namespace

DEF_GM( return new ConvexPathsGM; )
