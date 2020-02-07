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
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "src/core/SkGeometry.h"

static constexpr float kStrokeWidth = 40;
static constexpr int kCellSize = 200;

static const SkPoint kCubics[][4] = {
    {{122, 737}, {348, 553}, {403, 761}, {400, 760}},
    {{244, 520}, {244, 518}, {1141, 634}, {394, 688}},
    {{550, 194}, {138, 130}, {1035, 246}, {288, 300}},
    {{226, 733}, {556, 779}, {-43, 471}, {348, 683}},
    {{268, 204}, {492, 304}, {352, 23}, {433, 412}},
    {{172, 480}, {396, 580}, {256, 299}, {338, 677}},
    {{731, 340}, {318, 252}, {1026, -64}, {367, 265}},
    {{475, 708}, {62, 620}, {770, 304}, {220, 659}},
};

static SkRect calc_tight_cubic_bounds(const SkPoint P[4], int depth=5) {
    if (0 == depth) {
        SkRect bounds;
        bounds.fLeft = std::min(std::min(P[0].x(), P[1].x()), std::min(P[2].x(), P[3].x()));
        bounds.fTop = std::min(std::min(P[0].y(), P[1].y()), std::min(P[2].y(), P[3].y()));
        bounds.fRight = std::max(std::max(P[0].x(), P[1].x()), std::max(P[2].x(), P[3].x()));
        bounds.fBottom = std::max(std::max(P[0].y(), P[1].y()), std::max(P[2].y(), P[3].y()));
        return bounds;
    }

    SkPoint chopped[7];
    SkChopCubicAt(P, chopped, .5f);
    SkRect bounds = calc_tight_cubic_bounds(chopped, depth - 1);
    bounds.join(calc_tight_cubic_bounds(chopped+3, depth - 1));
    return bounds;
}

// This is a compilation of cubics that have given strokers grief. Feel free to add more.
class TrickyCubicStrokesGM : public skiagm::GM {
public:
    TrickyCubicStrokesGM() {}

protected:

    SkString onShortName() override {
        return SkString("trickycubicstrokes");
    }

    SkISize onISize() override {
        return SkISize::Make(3*kCellSize, 3*kCellSize);
    }

    void onOnceBeforeDraw() override {
        fStrokePaint.setAntiAlias(true);
        fStrokePaint.setStrokeWidth(kStrokeWidth);
        fStrokePaint.setColor(SK_ColorGREEN);
        fStrokePaint.setStyle(SkPaint::kStroke_Style);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorBLACK);

        for (size_t i = 0; i < SK_ARRAY_COUNT(kCubics); ++i) {
            this->drawStroke(canvas, kCubics[i],
                             SkRect::MakeXYWH((i%3) * kCellSize, (i/3) * kCellSize, kCellSize,
                                              kCellSize));
        }
    }

    void drawStroke(SkCanvas* canvas, const SkPoint P[4], const SkRect& location) {
        SkRect strokeBounds = calc_tight_cubic_bounds(P);
        strokeBounds.outset(kStrokeWidth, kStrokeWidth);

        SkMatrix matrix;
        matrix.setRectToRect(strokeBounds, location, SkMatrix::kCenter_ScaleToFit);

        SkPath path;
        path.moveTo(P[0]);
        path.cubicTo(P[1], P[2], P[3]);

        SkAutoCanvasRestore acr(canvas, true);
        canvas->concat(matrix);
        canvas->drawPath(path, fStrokePaint);
    }

private:
    SkPaint fStrokePaint;
    typedef GM INHERITED;
};

DEF_GM( return new TrickyCubicStrokesGM; )
