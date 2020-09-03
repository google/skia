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

static constexpr float kStrokeWidth = 30;
static constexpr int kCellSize = 200;
static constexpr int kNumCols = 4;
static constexpr int kNumRows = 4;

enum class CellFillMode {
    kStretch,
    kCenter
};

struct TrickyCubic {
    SkPoint fPoints[4];
    CellFillMode fFillMode;
    float fScale = 1;
};

static const TrickyCubic kTrickyCubics[] = {
    {{{122, 737}, {348, 553}, {403, 761}, {400, 760}}, CellFillMode::kStretch},
    {{{244, 520}, {244, 518}, {1141, 634}, {394, 688}}, CellFillMode::kStretch},
    {{{550, 194}, {138, 130}, {1035, 246}, {288, 300}}, CellFillMode::kStretch},
    {{{226, 733}, {556, 779}, {-43, 471}, {348, 683}}, CellFillMode::kStretch},
    {{{268, 204}, {492, 304}, {352, 23}, {433, 412}}, CellFillMode::kStretch},
    {{{172, 480}, {396, 580}, {256, 299}, {338, 677}}, CellFillMode::kStretch},
    {{{731, 340}, {318, 252}, {1026, -64}, {367, 265}}, CellFillMode::kStretch},
    {{{475, 708}, {62, 620}, {770, 304}, {220, 659}}, CellFillMode::kStretch},
    {{{0, 0}, {128, 128}, {128, 0}, {0, 128}}, CellFillMode::kCenter},  // Perfect cusp
    {{{0,.01f}, {128,127.999f}, {128,.01f}, {0,127.99f}}, CellFillMode::kCenter},  // Near-cusp
    {{{0,-.01f}, {128,128.001f}, {128,-.01f}, {0,128.001f}}, CellFillMode::kCenter},  // Near-cusp
    {{{0,0}, {0,-10}, {0,-10}, {0,10}}, CellFillMode::kCenter, 1.098283f},  // Flat line with 180
    {{{10,0}, {0,0}, {20,0}, {10,0}}, CellFillMode::kStretch},  // Flat line with 2 180s
    {{{39,-39}, {40,-40}, {40,-40}, {0,0}}, CellFillMode::kStretch},  // Flat diagonal with 180
    {{{40, 40}, {0, 0}, {200, 200}, {0, 0}}, CellFillMode::kStretch},  // Diag with an internal 180
    {{{0,0}, {1e-2f,0}, {-1e-2f,0}, {0,0}}, CellFillMode::kCenter},  // Circle
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

enum class FillMode {
    kCenter,
    kScale
};

// This is a compilation of cubics that have given strokers grief. Feel free to add more.
class TrickyCubicStrokesGM : public skiagm::GM {
public:
    TrickyCubicStrokesGM() {}

protected:

    SkString onShortName() override {
        return SkString("trickycubicstrokes");
    }

    SkISize onISize() override {
        return SkISize::Make(kNumCols * kCellSize, kNumRows * kCellSize);
    }

    void onOnceBeforeDraw() override {
        fStrokePaint.setAntiAlias(true);
        fStrokePaint.setStrokeWidth(kStrokeWidth);
        fStrokePaint.setColor(SK_ColorGREEN);
        fStrokePaint.setStyle(SkPaint::kStroke_Style);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorBLACK);

        for (size_t i = 0; i < SK_ARRAY_COUNT(kTrickyCubics); ++i) {
            const auto& trickyCubic = kTrickyCubics[i];
            SkPoint p[7];
            memcpy(p, trickyCubic.fPoints, sizeof(SkPoint) * 4);
            for (int j = 0; j < 4; ++j) {
                p[j] *= trickyCubic.fScale;
            }
            this->drawStroke(canvas, p, i, trickyCubic.fFillMode);
        }
    }

    void drawStroke(SkCanvas* canvas, const SkPoint p[4], int cellID, CellFillMode fillMode) {
        auto cellRect = SkRect::MakeXYWH((cellID % kNumCols) * kCellSize,
                                         (cellID / kNumCols) * kCellSize,
                                         kCellSize, kCellSize);

        SkRect strokeBounds = calc_tight_cubic_bounds(p);
        strokeBounds.outset(kStrokeWidth, kStrokeWidth);

        SkMatrix matrix;
        if (fillMode == CellFillMode::kStretch) {
            matrix.setRectToRect(strokeBounds, cellRect, SkMatrix::kCenter_ScaleToFit);
        } else {
            matrix.setTranslate(cellRect.x() + kStrokeWidth +
                                (cellRect.width() - strokeBounds.width()) / 2,
                                cellRect.y() + kStrokeWidth +
                                (cellRect.height() - strokeBounds.height()) / 2);
        }

        SkAutoCanvasRestore acr(canvas, true);
        canvas->concat(matrix);
        fStrokePaint.setStrokeWidth(kStrokeWidth / matrix.getMaxScale());
        canvas->drawPath(SkPath().moveTo(p[0]).cubicTo(p[1], p[2], p[3]), fStrokePaint);
    }

private:
    SkPaint fStrokePaint;
    using INHERITED = GM;
};

DEF_GM( return new TrickyCubicStrokesGM; )
