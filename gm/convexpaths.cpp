
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm.h"
#include "SkRandom.h"
#include "SkTArray.h"

namespace skiagm {

class ConvexPathsGM : public GM {
public:
    ConvexPathsGM() {
        this->setBGColor(0xFF000000);
        this->makePaths();
    }

protected:
    virtual SkString onShortName() {
        return SkString("convexpaths");
    }


    virtual SkISize onISize() {
        return make_isize(1200, 900);
    }

    void makePaths() {
        fPaths.push_back().addRect(0, 0,
                                   100 * SK_Scalar1, 100 * SK_Scalar1,
                                   SkPath::kCW_Direction);

        fPaths.push_back().addRect(0, 0,
                                   100 * SK_Scalar1, 100 * SK_Scalar1,
                                   SkPath::kCCW_Direction);

        fPaths.push_back().addCircle(50  * SK_Scalar1, 50  * SK_Scalar1,
                                     50  * SK_Scalar1, SkPath::kCW_Direction);

        fPaths.push_back().addCircle(50  * SK_Scalar1, 50  * SK_Scalar1,
                                     40  * SK_Scalar1, SkPath::kCCW_Direction);

        fPaths.push_back().addOval(SkRect::MakeXYWH(0, 0,
                                                    50 * SK_Scalar1,
                                                    100 * SK_Scalar1),
                                   SkPath::kCW_Direction);

        fPaths.push_back().addOval(SkRect::MakeXYWH(0, 0,
                                                    100 * SK_Scalar1,
                                                    50 * SK_Scalar1),
                                   SkPath::kCCW_Direction);

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

        fPaths.push_back().addRoundRect(SkRect::MakeXYWH(0, 0,
                                                         SK_Scalar1 * 100,
                                                         SK_Scalar1 * 100),
                                        20 * SK_Scalar1, 40 * SK_Scalar1,
                                        SkPath::kCCW_Direction);

        // shallow diagonals
        fPaths.push_back().lineTo(100 * SK_Scalar1, SK_Scalar1);
        fPaths.back().lineTo(98 * SK_Scalar1, 100 * SK_Scalar1);
        fPaths.back().lineTo(3 * SK_Scalar1, 96 * SK_Scalar1);

        /*
        It turns out arcTos are not automatically marked as convex and they
        may in fact be ever so slightly concave.
        fPaths.push_back().arcTo(SkRect::MakeXYWH(0, 0,
                                                  50 * SK_Scalar1,
                                                  100 * SK_Scalar1),
                                 25 * SK_Scalar1,  130 * SK_Scalar1, false);
        */

        // cubics
        fPaths.push_back().cubicTo( 1 * SK_Scalar1,  1 * SK_Scalar1,
                                   10 * SK_Scalar1,  90 * SK_Scalar1,
                                    0 * SK_Scalar1, 100 * SK_Scalar1);
        fPaths.push_back().cubicTo(100 * SK_Scalar1,  50 * SK_Scalar1,
                                    20 * SK_Scalar1, 100 * SK_Scalar1,
                                     0 * SK_Scalar1,   0 * SK_Scalar1);

        // point degenerate
        fPaths.push_back().moveTo(50 * SK_Scalar1, 50 * SK_Scalar1);
        fPaths.back().lineTo(50 * SK_Scalar1, 50 * SK_Scalar1);
        
        fPaths.push_back().moveTo(50 * SK_Scalar1, 50 * SK_Scalar1);
        fPaths.back().quadTo(50 * SK_Scalar1, 50 * SK_Scalar1,
                             50 * SK_Scalar1, 50 * SK_Scalar1);
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
    }

    virtual void onDraw(SkCanvas* canvas) {

    SkPaint paint;
    paint.setAntiAlias(true);
    SkRandom rand;
    canvas->translate(20 * SK_Scalar1, 20 * SK_Scalar1);
    for (int i = 0; i < fPaths.count(); ++i) {
        canvas->save();
        // position the path, and make it at off-integer coords.
        canvas->translate(SK_Scalar1 * 200 * (i % 5) + SK_Scalar1 / 4,
                          SK_Scalar1 * 200 * (i / 5) + 3 * SK_Scalar1 / 4);
        SkColor color = rand.nextU();
        color |= 0xff000000;
        paint.setColor(color);
        SkASSERT(fPaths[i].isConvex());
        canvas->drawPath(fPaths[i], paint);
        canvas->restore();
    }
    }
    
private:
    typedef GM INHERITED;
    SkTArray<SkPath> fPaths;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new ConvexPathsGM; }
static GMRegistry reg(MyFactory);

}

