/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkRandom.h"
#include "SkRRect.h"

namespace skiagm {

// Test out various combinations of nested rects, ovals and rrects.
class NestedGM : public GM {
public:
    NestedGM(bool doAA) : fDoAA(doAA) {
        this->setBGColor(0xFFDDDDDD);
    }

protected:
    virtual SkString onShortName() SK_OVERRIDE {
        SkString name("nested");
        if (fDoAA) {
            name.append("_aa");
        } else {
            name.append("_bw");
        }
        return name;
    }

    virtual SkISize onISize() SK_OVERRIDE {
        return make_isize(kImageWidth, kImageHeight);
    }

    enum Shapes {
        kRect_Shape = 0,
        kRRect_Shape,
        kOval_Shape,
        kShapeCount
    };

    static void AddShape(SkPath* path, const SkRect& rect, Shapes shape, SkPath::Direction dir) {
        switch (shape) {
            case kRect_Shape:
                path->addRect(rect, dir);
                break;
            case kRRect_Shape: {
                SkRRect rr;
                rr.setRectXY(rect, 5, 5);
                path->addRRect(rr, dir);
                break;
                }
            case kOval_Shape:
                path->addOval(rect, dir);
                break;
            default:
                break;
        }
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {

        SkPaint shapePaint;
        shapePaint.setColor(SK_ColorBLACK);
        shapePaint.setAntiAlias(fDoAA);

        SkRect outerRect = SkRect::MakeWH(40, 40);

        SkRect innerRects[] = {
            { 10, 10, 30, 30 },     // small
            { .5f, 18, 4.5f, 22 }   // smaller and offset to left
        };

        // draw a background pattern to make transparency errors more apparent
        SkRandom rand;

        for (int y = 0; y < kImageHeight; y += 10) {
            for (int x = 0; x < kImageWidth; x += 10) {
                SkRect r = SkRect::MakeXYWH(SkIntToScalar(x),
                                            SkIntToScalar(y),
                                            10, 10);
                SkPaint p;
                p.setColor(rand.nextU() | 0xFF000000);
                canvas->drawRect(r, p);
            }
        }

        canvas->translate(2, 2);
        for (int outerShape = 0; outerShape < kShapeCount; ++outerShape) {
            canvas->save();
            for (int innerShape = 0; innerShape < kShapeCount; ++innerShape) {
                for (size_t innerRect = 0; innerRect < SK_ARRAY_COUNT(innerRects); ++innerRect) {
                    SkPath path;

                    AddShape(&path, outerRect, (Shapes) outerShape, SkPath::kCW_Direction);
                    AddShape(&path, innerRects[innerRect], (Shapes) innerShape,
                             SkPath::kCCW_Direction);

                    canvas->drawPath(path, shapePaint);
                    canvas->translate(45, 0);
                }
            }
            canvas->restore();
            canvas->translate(0, 45);
        }

    }

private:
    static const int kImageWidth = 269;
    static const int kImageHeight = 134;

    bool fDoAA;

    typedef GM INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_GM( return new NestedGM(true); )
DEF_GM( return new NestedGM(false); )


}
