/*
 * Copyright 2018 Google Inc.
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
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/utils/SkRandom.h"
#include "tools/ToolUtils.h"

#include <array>
#include <vector>

namespace skiagm {

static constexpr int kPadSize = 20;
static constexpr int kBoxSize = 100;
static constexpr SkPoint kJitters[] = {{0, 0}, {.5f, .5f}, {2/3.f, 1/3.f}};

// Tests various corners of different angles falling on the same pixel, particularly to ensure
// analytic AA is working properly.
class SharedCornersGM : public GM {
public:
    SharedCornersGM() { this->setBGColor(ToolUtils::color_to_565(0xFF1A65D7)); }

protected:
    SkString onShortName() override {
        return SkString("sharedcorners");
    }

    SkISize onISize() override {
        constexpr int numRows = 3 * 2;
        constexpr int numCols = (1 + SK_ARRAY_COUNT(kJitters)) * 2;
        return SkISize::Make(numCols * (kBoxSize + kPadSize) + kPadSize,
                             numRows * (kBoxSize + kPadSize) + kPadSize);
    }

    void onOnceBeforeDraw() override {
        fFillPaint.setColor(SK_ColorWHITE);
        fFillPaint.setAntiAlias(true);

        fWireFramePaint = fFillPaint;
        fWireFramePaint.setStyle(SkPaint::kStroke_Style);

    }

    void onDraw(SkCanvas* canvas) override {
        canvas->translate(kPadSize, kPadSize);
        canvas->save();

        // Adjacent rects.
        this->drawTriangleBoxes(canvas,
                {{0,  0}, {40,  0}, {80,  0}, {120,  0},
                 {0, 20}, {40, 20}, {80, 20}, {120, 20},
                          {40, 40}, {80, 40},
                          {40, 60}, {80, 60}},
                {{{0, 1, 4}}, {{1, 5, 4}},
                 {{5, 1, 6}}, {{1, 2, 6}},
                 {{2, 3, 6}}, {{3, 7, 6}},
                 {{8, 5, 9}}, {{5, 6, 9}},
                 {{10, 8, 11}}, {{8, 9, 11}}});

        // Obtuse angles.
        this->drawTriangleBoxes(canvas,
                {{ 0, 0}, {10, 0}, {20, 0},
                 { 0, 2},          {20, 2},
                          {10, 4},
                 { 0, 6},          {20, 6},
                 { 0, 8}, {10, 8}, {20, 8}},
                {{{3, 1, 4}}, {{4, 5, 3}}, {{6, 5, 7}}, {{7, 9, 6}},
                 {{0, 1, 3}}, {{1, 2, 4}},
                 {{3, 5, 6}}, {{5, 4, 7}},
                 {{6, 9, 8}}, {{9, 7, 10}}});

        canvas->restore();
        canvas->translate((kBoxSize + kPadSize) * 4, 0);

        // Right angles.
        this->drawTriangleBoxes(canvas,
                {{0, 0}, {-1, 0}, {0, -1}, {1, 0}, {0, 1}},
                {{{0, 1, 2}}, {{0, 2, 3}}, {{0, 3, 4}}, {{0, 4, 1}}});

        // Acute angles.
        SkRandom rand;
        std::vector<SkPoint> pts;
        std::vector<std::array<int, 3>> indices;
        SkScalar theta = 0;
        pts.push_back({0, 0});
        while (theta < 2*SK_ScalarPI) {
            pts.push_back({SkScalarCos(theta), SkScalarSin(theta)});
            if (pts.size() > 2) {
                indices.push_back({{0, (int)pts.size() - 2, (int)pts.size() - 1}});
            }
            theta += rand.nextRangeF(0, SK_ScalarPI/3);
        }
        indices.push_back({{0, (int)pts.size() - 1, 1}});
        this->drawTriangleBoxes(canvas, pts, indices);
    }

    void drawTriangleBoxes(SkCanvas* canvas, const std::vector<SkPoint>& points,
                           const std::vector<std::array<int, 3>>& triangles) {
        SkPath path;
        path.setFillType(SkPath::kEvenOdd_FillType);
        path.setIsVolatile(true);
        for (const std::array<int, 3>& triangle : triangles) {
            path.moveTo(points[triangle[0]]);
            path.lineTo(points[triangle[1]]);
            path.lineTo(points[triangle[2]]);
            path.close();
        }
        SkScalar scale = kBoxSize / SkTMax(path.getBounds().height(), path.getBounds().width());
        path.transform(SkMatrix::MakeScale(scale, scale));

        this->drawRow(canvas, path);
        canvas->translate(0, kBoxSize + kPadSize);

        SkMatrix rot;
        rot.setRotate(45, path.getBounds().centerX(), path.getBounds().centerY());
        path.transform(rot);
        this->drawRow(canvas, path);
        canvas->translate(0, kBoxSize + kPadSize);

        rot.setRotate(-45 - 69.38111f, path.getBounds().centerX(), path.getBounds().centerY());
        path.transform(rot);
        this->drawRow(canvas, path);
        canvas->translate(0, kBoxSize + kPadSize);
    }

    void drawRow(SkCanvas* canvas, const SkPath& path) {
        SkAutoCanvasRestore acr(canvas, true);
        const SkRect& bounds = path.getBounds();
        canvas->translate((kBoxSize - bounds.width()) / 2 - bounds.left(),
                          (kBoxSize - bounds.height()) / 2 - bounds.top());

        canvas->drawPath(path, fWireFramePaint);
        canvas->translate(kBoxSize + kPadSize, 0);

        for (SkPoint jitter : kJitters) {
            {
                SkAutoCanvasRestore acr(canvas, true);
                canvas->translate(jitter.x(), jitter.y());
                canvas->drawPath(path, fFillPaint);
            }
            canvas->translate(kBoxSize + kPadSize, 0);
        }
    }

    SkPaint fWireFramePaint;
    SkPaint fFillPaint;
};

DEF_GM(return new SharedCornersGM;)

}
