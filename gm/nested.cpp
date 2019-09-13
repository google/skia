/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/utils/SkRandom.h"

namespace skiagm {

// Test out various combinations of nested rects, ovals and rrects.
class NestedGM : public GM {
public:
    NestedGM(bool doAA, bool flipped) : fDoAA(doAA), fFlipped(flipped) {
        this->setBGColor(0xFFDDDDDD);
    }

protected:

    SkString onShortName() override {
        SkString name("nested");
        if (fFlipped) {
            name.append("_flipY");
        }
        if (fDoAA) {
            name.append("_aa");
        } else {
            name.append("_bw");
        }
        return name;
    }

    SkISize onISize() override {
        return SkISize::Make(kImageWidth, kImageHeight);
    }

    enum Shapes {
        kRect_Shape = 0,
        kRRect_Shape,
        kOval_Shape,
        kShapeCount
    };

    static void AddShape(SkPath* path, const SkRect& rect, Shapes shape, SkPathDirection dir) {
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

    void onDraw(SkCanvas* canvas) override {

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

        SkScalar xOff = 2, yOff = 2;
        for (int outerShape = 0; outerShape < kShapeCount; ++outerShape) {
            for (int innerShape = 0; innerShape < kShapeCount; ++innerShape) {
                for (size_t innerRect = 0; innerRect < SK_ARRAY_COUNT(innerRects); ++innerRect) {
                    SkPath path;

                    AddShape(&path, outerRect, (Shapes) outerShape, SkPathDirection::kCW);
                    AddShape(&path, innerRects[innerRect], (Shapes) innerShape,
                             SkPathDirection::kCCW);

                    canvas->save();
                    if (fFlipped) {
                        canvas->scale(1.0f, -1.0f);
                        canvas->translate(xOff, -yOff - 40.0f);
                    } else {
                        canvas->translate(xOff, yOff);
                    }

                    canvas->drawPath(path, shapePaint);
                    canvas->restore();

                    xOff += 45;
                }
            }

            xOff = 2;
            yOff += 45;
        }

    }

private:
    static constexpr int kImageWidth = 269;
    static constexpr int kImageHeight = 134;

    bool fDoAA;
    bool fFlipped;

    typedef GM INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_GM( return new NestedGM(/* doAA = */ true,  /* flipped = */ false); )
DEF_GM( return new NestedGM(/* doAA = */ false, /* flipped = */ false); )
DEF_GM( return new NestedGM(/* doAA = */ true,  /* flipped = */ true); )
DEF_GM( return new NestedGM(/* doAA = */ false, /* flipped = */ true); )

}
