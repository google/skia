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
#include "include/core/SkPoint.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/SkTArray.h"
#include "include/utils/SkRandom.h"

namespace skiagm {

// This GM tests a grab-bag of convex and concave polygons. They are triangles,
// trapezoid, diamond, polygons with lots of edges, several concave polygons...
// But rectangles are excluded.
class PolygonsGM: public GM {
public:
    PolygonsGM() {}

protected:

    SkString onShortName() override {
        return SkString("polygons");
    }

    SkISize onISize() override {
        int width = kNumPolygons * kCellSize + 40;
        int height = (kNumJoins * kNumStrokeWidths + kNumExtraStyles) * kCellSize + 40;
        return SkISize::Make(width, height);
    }

    // Construct all polygons
    void onOnceBeforeDraw() override {
        SkPoint p0[] = {{0, 0}, {60, 0}, {90, 40}};  // triangle
        SkPoint p1[] = {{0, 0}, {0, 40}, {60, 40}, {40, 0}};  // trapezoid
        SkPoint p2[] = {{0, 0}, {40, 40}, {80, 40}, {40, 0}};  // diamond
        SkPoint p3[] = {{10, 0}, {50, 0}, {60, 10}, {60, 30}, {50, 40},
                        {10, 40}, {0, 30}, {0, 10}};  // octagon
        SkPoint p4[32];  // circle-like polygons with 32-edges.
        SkPoint p5[] = {{0, 0}, {20, 20}, {0, 40}, {60, 20}};  // concave polygon with 4 edges
        SkPoint p6[] = {{0, 40}, {0, 30}, {15, 30}, {15, 20}, {30, 20},
                        {30, 10}, {45, 10}, {45, 0}, {60, 0}, {60, 40}};  // stairs-like polygon
        SkPoint p7[] = {{0, 20}, {20, 20}, {30, 0}, {40, 20}, {60, 20},
                        {45, 30}, {55, 50}, {30, 40}, {5, 50}, {15, 30}};  // five-point stars

        for (size_t i = 0; i < SK_ARRAY_COUNT(p4); ++i) {
            SkScalar angle = 2 * SK_ScalarPI * i / SK_ARRAY_COUNT(p4);
            p4[i].set(20 * SkScalarCos(angle) + 20, 20 * SkScalarSin(angle) + 20);
        }

        struct Polygons {
            SkPoint* fPoints;
            size_t fPointNum;
        } pgs[] = {
            { p0, SK_ARRAY_COUNT(p0) },
            { p1, SK_ARRAY_COUNT(p1) },
            { p2, SK_ARRAY_COUNT(p2) },
            { p3, SK_ARRAY_COUNT(p3) },
            { p4, SK_ARRAY_COUNT(p4) },
            { p5, SK_ARRAY_COUNT(p5) },
            { p6, SK_ARRAY_COUNT(p6) },
            { p7, SK_ARRAY_COUNT(p7) }
        };

        SkASSERT(SK_ARRAY_COUNT(pgs) == kNumPolygons);
        for (size_t pgIndex = 0; pgIndex < SK_ARRAY_COUNT(pgs); ++pgIndex) {
            SkPathBuilder b;
            b.moveTo(pgs[pgIndex].fPoints[0].fX,
                     pgs[pgIndex].fPoints[0].fY);
            for (size_t ptIndex = 1; ptIndex < pgs[pgIndex].fPointNum; ++ptIndex) {
                b.lineTo(pgs[pgIndex].fPoints[ptIndex].fX,
                         pgs[pgIndex].fPoints[ptIndex].fY);
            }
            b.close();
            fPolygons.push_back(b.detach());
        }
    }

    // Set the location for the current test on the canvas
    static void SetLocation(SkCanvas* canvas, int counter, int lineNum) {
        SkScalar x = SK_Scalar1 * kCellSize * (counter % lineNum) + 30 + SK_Scalar1 / 4;
        SkScalar y = SK_Scalar1 * kCellSize * (counter / lineNum) + 30 + 3 * SK_Scalar1 / 4;
        canvas->translate(x, y);
    }

    static void SetColorAndAlpha(SkPaint* paint, SkRandom* rand) {
        SkColor color = rand->nextU();
        color |= 0xff000000;
        paint->setColor(color);
        if (40 == paint->getStrokeWidth()) {
            paint->setAlpha(0xA0);
        }
    }

    void onDraw(SkCanvas* canvas) override {
        // Stroke widths are:
        // 0(may use hairline rendering), 10(common case for stroke-style)
        // 40(>= geometry width/height, make the contour filled in fact)
        constexpr int kStrokeWidths[] = {0, 10, 40};
        SkASSERT(kNumStrokeWidths == SK_ARRAY_COUNT(kStrokeWidths));

        constexpr SkPaint::Join kJoins[] = {
            SkPaint::kMiter_Join, SkPaint::kRound_Join, SkPaint::kBevel_Join
        };
        SkASSERT(kNumJoins == SK_ARRAY_COUNT(kJoins));

        int counter = 0;
        SkPaint paint;
        paint.setAntiAlias(true);

        SkRandom rand;
        // For stroke style painter
        paint.setStyle(SkPaint::kStroke_Style);
        for (int join = 0; join < kNumJoins; ++join) {
            for (int width = 0; width < kNumStrokeWidths; ++width) {
                for (int i = 0; i < fPolygons.count(); ++i) {
                    canvas->save();
                    SetLocation(canvas, counter, fPolygons.count());

                    SetColorAndAlpha(&paint, &rand);
                    paint.setStrokeJoin(kJoins[join]);
                    paint.setStrokeWidth(SkIntToScalar(kStrokeWidths[width]));

                    canvas->drawPath(fPolygons[i], paint);
                    canvas->restore();
                    ++counter;
                }
            }
        }

        // For stroke-and-fill style painter and fill style painter
        constexpr SkPaint::Style kStyles[] = {
            SkPaint::kStrokeAndFill_Style, SkPaint::kFill_Style
        };
        SkASSERT(kNumExtraStyles == SK_ARRAY_COUNT(kStyles));

        paint.setStrokeJoin(SkPaint::kMiter_Join);
        paint.setStrokeWidth(SkIntToScalar(20));
        for (int style = 0; style < kNumExtraStyles; ++style) {
            paint.setStyle(kStyles[style]);
            for (int i = 0; i < fPolygons.count(); ++i) {
                canvas->save();
                SetLocation(canvas, counter, fPolygons.count());
                SetColorAndAlpha(&paint, &rand);
                canvas->drawPath(fPolygons[i], paint);
                canvas->restore();
                ++counter;
            }
        }
    }

private:
    inline static constexpr int kNumPolygons = 8;
    inline static constexpr int kCellSize = 100;
    inline static constexpr int kNumExtraStyles = 2;
    inline static constexpr int kNumStrokeWidths = 3;
    inline static constexpr int kNumJoins = 3;

    SkTArray<SkPath> fPolygons;
    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new PolygonsGM;)

// see crbug.com/1197461
DEF_SIMPLE_GM(conjoined_polygons, canvas, 400, 400) {
    SkPathBuilder b;
    b.moveTo(0.f, 120.f);
    b.lineTo(0.f, 0.f);
    b.lineTo(50.f, 330.f);
    b.lineTo(90.f, 0.f);
    b.lineTo(340.f, 0.f);
    b.lineTo(90.f, 330.f);
    b.lineTo(50.f, 330.f);
    b.close();

    SkPaint paint;
    paint.setAntiAlias(true);
    canvas->drawPath(b.detach(), paint);
}

}  // namespace skiagm
