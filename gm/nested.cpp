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
#include "include/core/SkPathBuilder.h"
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

    static void AddShape(SkPathBuilder* b, const SkRect& rect, Shapes shape, SkPathDirection dir) {
        switch (shape) {
            case kRect_Shape:
                b->addRect(rect, dir);
                break;
            case kRRect_Shape: {
                SkRRect rr;
                rr.setRectXY(rect, 5, 5);
                b->addRRect(rr, dir);
                break;
                }
            case kOval_Shape:
                b->addOval(rect, dir);
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
                    SkPathBuilder builder;

                    AddShape(&builder, outerRect, (Shapes) outerShape, SkPathDirection::kCW);
                    AddShape(&builder, innerRects[innerRect], (Shapes) innerShape,
                             SkPathDirection::kCCW);

                    canvas->save();
                    if (fFlipped) {
                        canvas->scale(1.0f, -1.0f);
                        canvas->translate(xOff, -yOff - 40.0f);
                    } else {
                        canvas->translate(xOff, yOff);
                    }

                    canvas->drawPath(builder.detach(), shapePaint);
                    canvas->restore();

                    xOff += 45;
                }
            }

            xOff = 2;
            yOff += 45;
        }

    }

private:
    inline static constexpr int kImageWidth = 269;
    inline static constexpr int kImageHeight = 134;

    bool fDoAA;
    bool fFlipped;

    using INHERITED = GM;
};

///////////////////////////////////////////////////////////////////////////////

DEF_GM( return new NestedGM(/* doAA = */ true,  /* flipped = */ false); )
DEF_GM( return new NestedGM(/* doAA = */ false, /* flipped = */ false); )
DEF_GM( return new NestedGM(/* doAA = */ true,  /* flipped = */ true); )
DEF_GM( return new NestedGM(/* doAA = */ false, /* flipped = */ true); )

DEF_SIMPLE_GM(nested_hairline_square, canvas, 64, 64) {
    // See crbug.com/1234194 - This should draw 1 row of 3 stroked squares, with a second 0.5px
    // shifted row of squares below it.
    auto drawEllipses = [&]() {
        canvas->save();
        // Originally the SVG string "M5,14H0V9h5V14Z M1,13h3v-3H1V13Z" but that just specifies a
        // 5px wide square outside a 3px wide square.
        SkPath square;
        square.addRect(SkRect::MakeLTRB(0.f, 9.f, 5.f, 14.f));
        square.addRect(SkRect::MakeLTRB(1.f, 10.f, 4.f, 13.f), SkPathDirection::kCCW);

        // From the bug, SVG viewbox was (0, 0, 24, 24), so the above coordinates are relative to
        // that, but the svg was then the child of a div that was 16x16, so it's scaled down. This
        // converts the 1px wide nested rects into subpixel nested rects.
        canvas->scale(16.f / 24.f, 16.f / 24.f);

        SkPaint paint;
        paint.setColor(SkColorSetARGB(255, 70, 70, 70));
        paint.setAntiAlias(true);

        // The original SVG drew 3 separate paths, but these were just translations of the original
        // path baked into a path string.
        canvas->drawPath(square, paint);
        canvas->translate(10.f, 0.f);
        canvas->drawPath(square, paint);
        canvas->translate(10.f, 0.f);
        canvas->drawPath(square, paint);

        canvas->restore();
    };

    drawEllipses();
    canvas->translate(0.5f, 16.f);
    drawEllipses();
}

}  // namespace skiagm
