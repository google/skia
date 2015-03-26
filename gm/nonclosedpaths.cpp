/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkPath.h"

namespace skiagm {

// This GM tests a grab-bag of non-closed paths. All these paths look like
// closed rects, but they don't call path.close(). Depending on the stroke
// settings these slightly different paths give widely different results.
class NonClosedPathsGM: public GM {
public:
    NonClosedPathsGM() {}

    enum ClosureType {
        TotallyNonClosed,  // The last point doesn't coincide with the first one in the contour.
                           // The path looks not closed at all.

        FakeCloseCorner,   // The last point coincides with the first one at a corner.
                           // The path looks closed, but final rendering has 2 ends with cap.

        FakeCloseMiddle,   // The last point coincides with the first one in the middle of a line.
                           // The path looks closed, and the final rendering looks closed too.

        kClosureTypeCount
    };

protected:

    SkString onShortName() override {
        return SkString("nonclosedpaths");
    }

    // 12 * 18 + 3 cases, every case is 100 * 100 pixels.
    SkISize onISize() override {
        return SkISize::Make(1220, 1920);
    }

    // Use rect-like geometry for non-closed path, for right angles make it
    // easier to show the visual difference of lineCap and lineJoin.
    static void MakePath(SkPath* path, ClosureType type) {
        if (FakeCloseMiddle == type) {
            path->moveTo(30, 50);
            path->lineTo(30, 30);
        } else {
            path->moveTo(30, 30);
        }
        path->lineTo(70, 30);
        path->lineTo(70, 70);
        path->lineTo(30, 70);
        path->lineTo(30, 50);
        if (FakeCloseCorner == type) {
            path->lineTo(30, 30);
        }
    }

    // Set the location for the current test on the canvas
    static void SetLocation(SkCanvas* canvas, int counter, int lineNum) {
        SkScalar x = SK_Scalar1 * 100 * (counter % lineNum) + 10 + SK_Scalar1 / 4;
        SkScalar y = SK_Scalar1 * 100 * (counter / lineNum) + 10 + 3 * SK_Scalar1 / 4;
        canvas->translate(x, y);
    }

    void onDraw(SkCanvas* canvas) override {
        // Stroke widths are:
        // 0(may use hairline rendering), 10(common case for stroke-style)
        // 40 and 50(>= geometry width/height, make the contour filled in fact)
        static const int kStrokeWidth[] = {0, 10, 40, 50};
        int numWidths = SK_ARRAY_COUNT(kStrokeWidth);

        static const SkPaint::Style kStyle[] = {
            SkPaint::kStroke_Style, SkPaint::kStrokeAndFill_Style
        };

        static const SkPaint::Cap kCap[] = {
            SkPaint::kButt_Cap, SkPaint::kRound_Cap, SkPaint::kSquare_Cap
        };

        static const SkPaint::Join kJoin[] = {
            SkPaint::kMiter_Join, SkPaint::kRound_Join, SkPaint::kBevel_Join
        };

        static const ClosureType kType[] = {
            TotallyNonClosed, FakeCloseCorner, FakeCloseMiddle
        };

        int counter = 0;
        SkPaint paint;
        paint.setAntiAlias(true);

        // For stroke style painter and fill-and-stroke style painter
        for (size_t type = 0; type < kClosureTypeCount; ++type) {
            for (size_t style = 0; style < SK_ARRAY_COUNT(kStyle); ++style) {
                for (size_t cap = 0; cap < SK_ARRAY_COUNT(kCap); ++cap) {
                    for (size_t join = 0; join < SK_ARRAY_COUNT(kJoin); ++join) {
                        for (int width = 0; width < numWidths; ++width) {
                            canvas->save();
                            SetLocation(canvas, counter, SkPaint::kJoinCount * numWidths);

                            SkPath path;
                            MakePath(&path, kType[type]);

                            paint.setStyle(kStyle[style]);
                            paint.setStrokeCap(kCap[cap]);
                            paint.setStrokeJoin(kJoin[join]);
                            paint.setStrokeWidth(SkIntToScalar(kStrokeWidth[width]));

                            canvas->drawPath(path, paint);
                            canvas->restore();
                            ++counter;
                        }
                    }
                }
            }
        }

        // For fill style painter
        paint.setStyle(SkPaint::kFill_Style);
        for (size_t type = 0; type < kClosureTypeCount; ++type) {
            canvas->save();
            SetLocation(canvas, counter, SkPaint::kJoinCount * numWidths);

            SkPath path;
            MakePath(&path, kType[type]);

            canvas->drawPath(path, paint);
            canvas->restore();
            ++counter;
        }
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new NonClosedPathsGM;)

}
