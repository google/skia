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
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrDirectContext.h"
#include "src/base/SkRandom.h"
#include "src/core/SkGeometry.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrDrawingManager.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"

static constexpr float kStrokeWidth = 30;
static constexpr int kCellSize = 200;
static constexpr int kNumCols = 5;
static constexpr int kNumRows = 5;
static constexpr int kTestWidth = kNumCols * kCellSize;
static constexpr int kTestHeight = kNumRows * kCellSize;

enum class CellFillMode {
    kStretch,
    kCenter
};

struct TrickyCubic {
    SkPoint fPoints[4];
    int fNumPts;
    CellFillMode fFillMode;
    float fScale = 1;
};

// This is a compilation of cubics that have given strokers grief. Feel free to add more.
static const TrickyCubic kTrickyCubics[] = {
    {{{122, 737}, {348, 553}, {403, 761}, {400, 760}}, 4, CellFillMode::kStretch},
    {{{244, 520}, {244, 518}, {1141, 634}, {394, 688}}, 4, CellFillMode::kStretch},
    {{{550, 194}, {138, 130}, {1035, 246}, {288, 300}}, 4, CellFillMode::kStretch},
    {{{226, 733}, {556, 779}, {-43, 471}, {348, 683}}, 4, CellFillMode::kStretch},
    {{{268, 204}, {492, 304}, {352, 23}, {433, 412}}, 4, CellFillMode::kStretch},
    {{{172, 480}, {396, 580}, {256, 299}, {338, 677}}, 4, CellFillMode::kStretch},
    {{{731, 340}, {318, 252}, {1026, -64}, {367, 265}}, 4, CellFillMode::kStretch},
    {{{475, 708}, {62, 620}, {770, 304}, {220, 659}}, 4, CellFillMode::kStretch},
    {{{0, 0}, {128, 128}, {128, 0}, {0, 128}}, 4, CellFillMode::kCenter},  // Perfect cusp
    {{{0,.01f}, {128,127.999f}, {128,.01f}, {0,127.99f}}, 4, CellFillMode::kCenter},  // Near-cusp
    {{{0,-.01f}, {128,128.001f}, {128,-.01f}, {0,128.001f}}, 4, CellFillMode::kCenter}, // Near-cusp
    {{{0,0}, {0,-10}, {0,-10}, {0,10}}, 4, CellFillMode::kCenter, 1.098283f},  // Flat line with 180
    {{{10,0}, {0,0}, {20,0}, {10,0}}, 4, CellFillMode::kStretch},  // Flat line with 2 180s
    {{{39,-39}, {40,-40}, {40,-40}, {0,0}}, 4, CellFillMode::kStretch},  // Flat diagonal with 180
    {{{39,-39}, {40,-40}, {37,-39}, {0,0}}, 4, CellFillMode::kStretch},  // Near-flat diagonal
    {{{40, 40}, {0, 0}, {200, 200}, {0, 0}}, 4, CellFillMode::kStretch},  // Diag w/ an internal 180
    {{{0,0}, {1e-2f,0}, {-1e-2f,0}, {0,0}}, 4, CellFillMode::kCenter},  // Circle
    {{{400.75f,100.05f}, {400.75f,100.05f}, {100.05f,300.95f}, {100.05f,300.95f}}, 4,
     CellFillMode::kStretch},  // Flat line with no turns
    {{{0.5f,0}, {0,0}, {20,0}, {10,0}}, 4, CellFillMode::kStretch},  // Flat line with 2 180s
    {{{10,0}, {0,0}, {10,0}, {10,0}}, 4, CellFillMode::kStretch},  // Flat line with a 180
    {{{1,1}, {2,1}, {1,1}, {1, std::numeric_limits<float>::quiet_NaN()}}, 3,
     CellFillMode::kStretch},  // Flat QUAD with a cusp
    {{{1,1}, {100,1}, {25,1}, {.3f, std::numeric_limits<float>::quiet_NaN()}}, 3,
     CellFillMode::kStretch},  // Flat CONIC with a cusp
    {{{1,1}, {100,1}, {25,1}, {1.5f, std::numeric_limits<float>::quiet_NaN()}}, 3,
     CellFillMode::kStretch},  // Flat CONIC with a cusp
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

static SkPoint lerp(const SkPoint& a, const SkPoint& b, float T) {
    SkASSERT(1 != T);  // The below does not guarantee lerp(a, b, 1) === b.
    return (b - a) * T + a;
}

enum class FillMode {
    kCenter,
    kScale
};

static void draw_test(SkCanvas* canvas, SkPaint::Cap cap, SkPaint::Join join) {
    SkRandom rand;

    canvas->clear(SK_ColorBLACK);

    SkPaint strokePaint;
    strokePaint.setAntiAlias(true);
    strokePaint.setStrokeWidth(kStrokeWidth);
    strokePaint.setStyle(SkPaint::kStroke_Style);
    strokePaint.setStrokeCap(cap);
    strokePaint.setStrokeJoin(join);

    for (size_t i = 0; i < std::size(kTrickyCubics); ++i) {
        auto [originalPts, numPts, fillMode, scale] = kTrickyCubics[i];

        SkASSERT(numPts <= 4);
        SkPoint p[4];
        memcpy(p, originalPts, sizeof(SkPoint) * numPts);
        for (int j = 0; j < numPts; ++j) {
            p[j] *= scale;
        }
        float w = originalPts[3].fX;

        auto cellRect = SkRect::MakeXYWH((i % kNumCols) * kCellSize, (i / kNumCols) * kCellSize,
                                         kCellSize, kCellSize);

        SkRect strokeBounds;
        if (numPts == 4) {
            strokeBounds = calc_tight_cubic_bounds(p);
        } else {
            SkASSERT(numPts == 3);
            SkPoint asCubic[4] = {p[0], lerp(p[0], p[1], 2/3.f), lerp(p[1], p[2], 1/3.f), p[2]};
            strokeBounds = calc_tight_cubic_bounds(asCubic);
        }
        strokeBounds.outset(kStrokeWidth, kStrokeWidth);

        SkMatrix matrix;
        if (fillMode == CellFillMode::kStretch) {
            matrix = SkMatrix::RectToRect(strokeBounds, cellRect, SkMatrix::kCenter_ScaleToFit);
        } else {
            matrix.setTranslate(cellRect.x() + kStrokeWidth +
                                (cellRect.width() - strokeBounds.width()) / 2,
                                cellRect.y() + kStrokeWidth +
                                (cellRect.height() - strokeBounds.height()) / 2);
        }

        SkAutoCanvasRestore acr(canvas, true);
        canvas->concat(matrix);
        strokePaint.setStrokeWidth(kStrokeWidth / matrix.getMaxScale());
        strokePaint.setColor(rand.nextU() | 0xff808080);
        SkPath path = SkPath().moveTo(p[0]);
        if (numPts == 4) {
            path.cubicTo(p[1], p[2], p[3]);
        } else if (w == 1) {
            SkASSERT(numPts == 3);
            path.quadTo(p[1], p[2]);
        } else {
            SkASSERT(numPts == 3);
            path.conicTo(p[1], p[2], w);
        }
        canvas->drawPath(path, strokePaint);
    }
}

DEF_SIMPLE_GM(trickycubicstrokes, canvas, kTestWidth, kTestHeight) {
    draw_test(canvas, SkPaint::kButt_Cap, SkPaint::kMiter_Join);
}

DEF_SIMPLE_GM(trickycubicstrokes_roundcaps, canvas, kTestWidth, kTestHeight) {
    draw_test(canvas, SkPaint::kRound_Cap, SkPaint::kRound_Join);
}
