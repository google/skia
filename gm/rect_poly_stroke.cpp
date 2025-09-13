/*
 * Copyright 2025 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"

using DrawRectProc = void(SkCanvas*, const SkRect&, const SkPaint&);

static void draw_rect_as_rect_proc(SkCanvas* canvas, const SkRect& rect, const SkPaint& paint) {
    canvas->drawRect(rect, paint);
}
static void draw_rect_with_path_proc(SkCanvas* canvas, const SkRect& rect, const SkPaint& paint) {
    canvas->drawPath(SkPath::Rect(rect), paint);
}

constexpr int GM_WIDTH = 1150, GM_HEIGHT = 920;

DEF_SIMPLE_GM(rect_poly_stroke, canvas, GM_WIDTH, GM_HEIGHT) {
    struct DrawRec {
        DrawRectProc* proc;
        SkColor       color;
    };
    DrawRec const recs[] = {
        {draw_rect_as_rect_proc, SK_ColorBLACK},
        {draw_rect_with_path_proc, 0xFF000088},
    };

    constexpr float W = 100, H = 80;
    const SkRect rects[] = {
        {0, 0, W, H},
        {0, 0, W, 0},
        {0, 0, 0, H},
        {0, 0, 0, 0},   // we don't expect this to draw anything
    };
    constexpr float spacing = 150;

    const float degrees[] = { 0, -30 };

    constexpr float thickness = 20;

    const SkPaint::Join joins[] = {
        SkPaint::kMiter_Join,
        SkPaint::kRound_Join,
        SkPaint::kBevel_Join,
    };

    canvas->translate(30, 50);

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStroke(true);
    paint.setStrokeWidth(thickness);
    for (auto j : joins) {
        paint.setStrokeJoin(j);

        canvas->save();
        for (auto r : rects) {
            for (auto angle : degrees) {
                canvas->save();
                for (auto rec : recs) {
                    canvas->save();
                    canvas->rotate(angle, r.centerX(), r.centerY());
                    {
                        paint.setStrokeWidth(thickness);
                        paint.setColor(rec.color);
                        rec.proc(canvas, r, paint);

                        paint.setStrokeWidth(0);
                        paint.setColor(SK_ColorGREEN);
                        rec.proc(canvas, r, paint);
                    }
                    canvas->restore();
                    canvas->translate(0, spacing);
                }
                canvas->restore();
                canvas->translate(spacing, 0);
            }
        }
        canvas->restore();
        canvas->translate(0, 2*spacing);
    }
}
