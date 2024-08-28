/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/gpu/ganesh/GrContextOptions.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "src/base/SkRandom.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrDrawingManager.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/ops/TessellationPathRenderer.h"

static constexpr float kStrokeWidth = 100;
static constexpr int kTestWidth = 120 * 4;
static constexpr int kTestHeight = 120 * 3 + 140;

static void draw_strokes(SkCanvas* canvas, SkRandom* rand, const SkPath& path,
                         const SkPath& cubic) {
    SkPaint strokePaint;
    strokePaint.setAntiAlias(true);
    strokePaint.setStrokeWidth(kStrokeWidth);
    strokePaint.setStyle(SkPaint::kStroke_Style);

    SkAutoCanvasRestore arc(canvas, true);
    strokePaint.setStrokeJoin(SkPaint::kBevel_Join);
    strokePaint.setColor(rand->nextU() | 0xff808080);
    canvas->drawPath(path, strokePaint);

    canvas->translate(120, 0);
    strokePaint.setStrokeJoin(SkPaint::kRound_Join);
    strokePaint.setColor(rand->nextU() | 0xff808080);
    canvas->drawPath(path, strokePaint);

    canvas->translate(120, 0);
    strokePaint.setStrokeJoin(SkPaint::kMiter_Join);
    strokePaint.setColor(rand->nextU() | 0xff808080);
    canvas->drawPath(path, strokePaint);

    canvas->translate(120, 0);
    strokePaint.setColor(rand->nextU() | 0xff808080);
    canvas->drawPath(cubic, strokePaint);
}

static void draw_test(SkCanvas* canvas) {
    SkRandom rand;

    canvas->clear(SK_ColorBLACK);

    SkAutoCanvasRestore arc(canvas, true);
    canvas->translate(60, 60);

    draw_strokes(canvas, &rand,
            SkPath().lineTo(10,0).lineTo(10,10),
            SkPath().cubicTo(10,0, 10,0, 10,10));
    canvas->translate(0, 120);

    draw_strokes(canvas, &rand,
            SkPath().lineTo(0,-10).lineTo(0,10),
            SkPath().cubicTo(0,-10, 0,-10, 0,10));
    canvas->translate(0, 120);

    draw_strokes(canvas, &rand,
            SkPath().lineTo(0,-10).lineTo(10,-10).lineTo(10,10).lineTo(0,10),
            SkPath().cubicTo(0,-10, 10,10, 0,10));
    canvas->translate(0, 140);

    draw_strokes(canvas, &rand,
            SkPath().lineTo(0,-10).lineTo(10,-10).lineTo(10,0).lineTo(0,0),
            SkPath().cubicTo(0,-10, 10,0, 0,0));
    canvas->translate(0, 120);
}

DEF_SIMPLE_GM(widebuttcaps, canvas, kTestWidth, kTestHeight) {
    canvas->clear(SK_ColorBLACK);
    draw_test(canvas);
}
