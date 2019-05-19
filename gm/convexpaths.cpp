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
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/SkNoncopyable.h"
#include "include/private/SkTArray.h"
#include "include/utils/SkRandom.h"

class SkDoOnce : SkNoncopyable {
public:
    SkDoOnce() { fDidOnce = false; }

    bool needToDo() const { return !fDidOnce; }
    bool alreadyDone() const { return fDidOnce; }
    void accomplished() {
        SkASSERT(!fDidOnce);
        fDidOnce = true;
    }

private:
    bool fDidOnce;
};

namespace skiagm {

class ConvexPathsGM : public GM {
    SkDoOnce fOnce;
public:
    ConvexPathsGM() {
        this->setBGColor(0xFF000000);
    }

protected:

    virtual SkString onShortName() {
        return SkString("convexpaths");
    }


    virtual SkISize onISize() {
        return SkISize::Make(1200, 1100);
    }

    void makePaths() {
        if (fOnce.alreadyDone()) {
            return;
        }
        fOnce.accomplished();

        fPaths.push_back().moveTo(0, 0);
        fPaths.back().quadTo(50 * SK_Scalar1, 100 * SK_Scalar1,
                             0, 100 * SK_Scalar1);
        fPaths.back().lineTo(0, 0);

        fPaths.push_back().moveTo(0, 50 * SK_Scalar1);
        fPaths.back().quadTo(50 * SK_Scalar1, 0,
                             100 * SK_Scalar1, 50 * SK_Scalar1);
        fPaths.back().quadTo(50 * SK_Scalar1, 100 * SK_Scalar1,
                             0, 50 * SK_Scalar1);

        fPaths.push_back().addRect(0, 0,
                                   100 * SK_Scalar1, 100 * SK_Scalar1,
                                   SkPath::kCW_Direction);

        fPaths.push_back().addRect(0, 0,
                                   100 * SK_Scalar1, 100 * SK_Scalar1,
                                   SkPath::kCCW_Direction);

        fPaths.push_back().addCircle(50  * SK_Scalar1, 50  * SK_Scalar1,
                                     50  * SK_Scalar1, SkPath::kCW_Direction);


        fPaths.push_back().addOval(SkRect::MakeXYWH(0, 0,
                                                    50 * SK_Scalar1,
                                                    100 * SK_Scalar1),
                                   SkPath::kCW_Direction);

        fPaths.push_back().addOval(SkRect::MakeXYWH(0, 0,
                                                    100 * SK_Scalar1,
                                                    5 * SK_Scalar1),
                                   SkPath::kCCW_Direction);

        fPaths.push_back().addOval(SkRect::MakeXYWH(0, 0,
                                                    SK_Scalar1,
                                                    100 * SK_Scalar1),
                                                    SkPath::kCCW_Direction);

        fPaths.push_back().addRoundRect(SkRect::MakeXYWH(0, 0,
                                                         SK_Scalar1 * 100,
                                                         SK_Scalar1 * 100),
                                        40 * SK_Scalar1, 20 * SK_Scalar1,
                                        SkPath::kCW_Direction);

        // large number of points
        enum {
            kLength = 100,
            kPtsPerSide = (1 << 12),
        };
        fPaths.push_back().moveTo(0, 0);
        for (int i = 1; i < kPtsPerSide; ++i) { // skip the first point due to moveTo.
            fPaths.back().lineTo(kLength * SkIntToScalar(i) / kPtsPerSide, 0);
        }
        for (int i = 0; i < kPtsPerSide; ++i) {
            fPaths.back().lineTo(kLength, kLength * SkIntToScalar(i) / kPtsPerSide);
        }
        for (int i = kPtsPerSide; i > 0; --i) {
            fPaths.back().lineTo(kLength * SkIntToScalar(i) / kPtsPerSide, kLength);
        }
        for (int i = kPtsPerSide; i > 0; --i) {
            fPaths.back().lineTo(0, kLength * SkIntToScalar(i) / kPtsPerSide);
        }

        // shallow diagonals
        fPaths.push_back().lineTo(100 * SK_Scalar1, SK_Scalar1);
        fPaths.back().lineTo(98 * SK_Scalar1, 100 * SK_Scalar1);
        fPaths.back().lineTo(3 * SK_Scalar1, 96 * SK_Scalar1);

        fPaths.push_back().arcTo(SkRect::MakeXYWH(0, 0,
                                                  50 * SK_Scalar1,
                                                  100 * SK_Scalar1),
                                                  25 * SK_Scalar1,  130 * SK_Scalar1, false);

        // cubics
        fPaths.push_back().cubicTo( 1 * SK_Scalar1,  1 * SK_Scalar1,
                                   10 * SK_Scalar1,  90 * SK_Scalar1,
                                    0 * SK_Scalar1, 100 * SK_Scalar1);
        fPaths.push_back().cubicTo(100 * SK_Scalar1,  50 * SK_Scalar1,
                                    20 * SK_Scalar1, 100 * SK_Scalar1,
                                     0 * SK_Scalar1,   0 * SK_Scalar1);

        // path that has a cubic with a repeated first control point and
        // a repeated last control point.
        fPaths.push_back().moveTo(SK_Scalar1 * 10, SK_Scalar1 * 10);
        fPaths.back().cubicTo(10 * SK_Scalar1, 10 * SK_Scalar1,
                              10 * SK_Scalar1, 0,
                              20 * SK_Scalar1, 0);
        fPaths.back().lineTo(40 * SK_Scalar1, 0);
        fPaths.back().cubicTo(40 * SK_Scalar1, 0,
                              50 * SK_Scalar1, 0,
                              50 * SK_Scalar1, 10 * SK_Scalar1);

        // path that has two cubics with repeated middle control points.
        fPaths.push_back().moveTo(SK_Scalar1 * 10, SK_Scalar1 * 10);
        fPaths.back().cubicTo(10 * SK_Scalar1, 0,
                              10 * SK_Scalar1, 0,
                              20 * SK_Scalar1, 0);
        fPaths.back().lineTo(40 * SK_Scalar1, 0);
        fPaths.back().cubicTo(50 * SK_Scalar1, 0,
                              50 * SK_Scalar1, 0,
                              50 * SK_Scalar1, 10 * SK_Scalar1);

        // cubic where last three points are almost a line
        fPaths.push_back().moveTo(0, 228 * SK_Scalar1 / 8);
        fPaths.back().cubicTo(628 * SK_Scalar1 / 8, 82 * SK_Scalar1 / 8,
                              1255 * SK_Scalar1 / 8, 141 * SK_Scalar1 / 8,
                              1883 * SK_Scalar1 / 8, 202 * SK_Scalar1 / 8);

        // flat cubic where the at end point tangents both point outward.
        fPaths.push_back().moveTo(10 * SK_Scalar1, 0);
        fPaths.back().cubicTo(0, SK_Scalar1,
                              30 * SK_Scalar1, SK_Scalar1,
                              20 * SK_Scalar1, 0);

        // flat cubic where initial tangent is in, end tangent out
        fPaths.push_back().moveTo(0, 0 * SK_Scalar1);
        fPaths.back().cubicTo(10 * SK_Scalar1, SK_Scalar1,
                              30 * SK_Scalar1, SK_Scalar1,
                              20 * SK_Scalar1, 0);

        // flat cubic where initial tangent is out, end tangent in
        fPaths.push_back().moveTo(10 * SK_Scalar1, 0);
        fPaths.back().cubicTo(0, SK_Scalar1,
                              20 * SK_Scalar1, SK_Scalar1,
                              30 * SK_Scalar1, 0);

        // triangle where one edge is a degenerate quad
        fPaths.push_back().moveTo(8.59375f, 45 * SK_Scalar1);
        fPaths.back().quadTo(16.9921875f,   45 * SK_Scalar1,
                             31.25f,        45 * SK_Scalar1);
        fPaths.back().lineTo(100 * SK_Scalar1,              100 * SK_Scalar1);
        fPaths.back().lineTo(8.59375f,      45 * SK_Scalar1);

        // triangle where one edge is a quad with a repeated point
        fPaths.push_back().moveTo(0, 25 * SK_Scalar1);
        fPaths.back().lineTo(50 * SK_Scalar1, 0);
        fPaths.back().quadTo(50 * SK_Scalar1, 50 * SK_Scalar1, 50 * SK_Scalar1, 50 * SK_Scalar1);

        // triangle where one edge is a cubic with a 2x repeated point
        fPaths.push_back().moveTo(0, 25 * SK_Scalar1);
        fPaths.back().lineTo(50 * SK_Scalar1, 0);
        fPaths.back().cubicTo(50 * SK_Scalar1, 0,
                              50 * SK_Scalar1, 50 * SK_Scalar1,
                              50 * SK_Scalar1, 50 * SK_Scalar1);

        // triangle where one edge is a quad with a nearly repeated point
        fPaths.push_back().moveTo(0, 25 * SK_Scalar1);
        fPaths.back().lineTo(50 * SK_Scalar1, 0);
        fPaths.back().quadTo(50 * SK_Scalar1, 49.95f,
                             50 * SK_Scalar1, 50 * SK_Scalar1);

        // triangle where one edge is a cubic with a 3x nearly repeated point
        fPaths.push_back().moveTo(0, 25 * SK_Scalar1);
        fPaths.back().lineTo(50 * SK_Scalar1, 0);
        fPaths.back().cubicTo(50 * SK_Scalar1, 49.95f,
                              50 * SK_Scalar1, 49.97f,
                              50 * SK_Scalar1, 50 * SK_Scalar1);

        // triangle where there is a point degenerate cubic at one corner
        fPaths.push_back().moveTo(0, 25 * SK_Scalar1);
        fPaths.back().lineTo(50 * SK_Scalar1, 0);
        fPaths.back().lineTo(50 * SK_Scalar1, 50 * SK_Scalar1);
        fPaths.back().cubicTo(50 * SK_Scalar1, 50 * SK_Scalar1,
                              50 * SK_Scalar1, 50 * SK_Scalar1,
                              50 * SK_Scalar1, 50 * SK_Scalar1);

        // point line
        fPaths.push_back().moveTo(50 * SK_Scalar1, 50 * SK_Scalar1);
        fPaths.back().lineTo(50 * SK_Scalar1, 50 * SK_Scalar1);

        // point quad
        fPaths.push_back().moveTo(50 * SK_Scalar1, 50 * SK_Scalar1);
        fPaths.back().quadTo(50 * SK_Scalar1, 50 * SK_Scalar1,
                             50 * SK_Scalar1, 50 * SK_Scalar1);

        // point cubic
        fPaths.push_back().moveTo(50 * SK_Scalar1, 50 * SK_Scalar1);
        fPaths.back().cubicTo(50 * SK_Scalar1, 50 * SK_Scalar1,
                              50 * SK_Scalar1, 50 * SK_Scalar1,
                              50 * SK_Scalar1, 50 * SK_Scalar1);

        // moveTo only paths
        fPaths.push_back().moveTo(0, 0);
        fPaths.back().moveTo(0, 0);
        fPaths.back().moveTo(SK_Scalar1, SK_Scalar1);
        fPaths.back().moveTo(SK_Scalar1, SK_Scalar1);
        fPaths.back().moveTo(10 * SK_Scalar1, 10 * SK_Scalar1);

        fPaths.push_back().moveTo(0, 0);
        fPaths.back().moveTo(0, 0);

        // line degenerate
        fPaths.push_back().lineTo(100 * SK_Scalar1, 100 * SK_Scalar1);
        fPaths.push_back().quadTo(100 * SK_Scalar1, 100 * SK_Scalar1, 0, 0);
        fPaths.push_back().quadTo(100 * SK_Scalar1, 100 * SK_Scalar1,
                                  50 * SK_Scalar1, 50 * SK_Scalar1);
        fPaths.push_back().quadTo(50 * SK_Scalar1, 50 * SK_Scalar1,
                                  100 * SK_Scalar1, 100 * SK_Scalar1);
        fPaths.push_back().cubicTo(0, 0,
                                   0, 0,
                                   100 * SK_Scalar1, 100 * SK_Scalar1);

        // skbug.com/8928
        fPaths.push_back().moveTo(16.875f, 192.594f);
        fPaths.back().cubicTo(45.625f, 192.594f, 74.375f, 192.594f, 103.125f, 192.594f);
        fPaths.back().cubicTo(88.75f, 167.708f, 74.375f, 142.823f, 60, 117.938f);
        fPaths.back().cubicTo(45.625f, 142.823f, 31.25f, 167.708f, 16.875f, 192.594f);
        fPaths.back().close();
        SkMatrix m;
        m.setAll(0.1f, 0, -1, 0, 0.115207f, -2.64977f, 0, 0, 1);
        fPaths.back().transform(m);

        // small circle. This is listed last so that it has device coords far
        // from the origin (small area relative to x,y values).
        fPaths.push_back().addCircle(0, 0, 1.2f);
    }

    virtual void onDraw(SkCanvas* canvas) {
        this->makePaths();

        SkPaint paint;
        paint.setAntiAlias(true);
        SkRandom rand;
        canvas->translate(20 * SK_Scalar1, 20 * SK_Scalar1);

        // As we've added more paths this has gotten pretty big. Scale the whole thing down.
        canvas->scale(2 * SK_Scalar1 / 3, 2 * SK_Scalar1 / 3);

        for (int i = 0; i < fPaths.count(); ++i) {
            canvas->save();
            // position the path, and make it at off-integer coords.
            canvas->translate(SK_Scalar1 * 200 * (i % 5) + SK_Scalar1 / 10,
                              SK_Scalar1 * 200 * (i / 5) + 9 * SK_Scalar1 / 10);
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

private:
    typedef GM INHERITED;
    SkTArray<SkPath> fPaths;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new ConvexPathsGM; )

}
