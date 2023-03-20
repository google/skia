/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathUtils.h"

static SkPath get_path() {
    SkPath path;
    path.setFillType(SkPathFillType::kWinding);
    path.moveTo(SkBits2Float(0x45034ec4), SkBits2Float(0x42e7fb80));  // 2100.92f, 115.991f
    path.quadTo(SkBits2Float(0x4500f46c),
                SkBits2Float(0x43333300),
                SkBits2Float(0x4500f46c),
                SkBits2Float(0x431f0ec0));  // 2063.28f, 179.199f, 2063.28f, 159.058f
    path.quadTo(SkBits2Float(0x4500f46c),
                SkBits2Float(0x430ad7c0),
                SkBits2Float(0x45019462),
                SkBits2Float(0x42fed580));  // 2063.28f, 138.843f, 2073.27f, 127.417f
    path.quadTo(SkBits2Float(0x45023458),
                SkBits2Float(0x42e7fb80),
                SkBits2Float(0x45034ec4),
                SkBits2Float(0x42e7fb80));  // 2083.27f, 115.991f, 2100.92f, 115.991f
    path.close();
    return path;
}

// Reproduces the underlying problem from skbug.com/12866.
// The path (part of a glyph) was being drawn stroked, and with a perspective matrix.
// The perspective matrix forces a very large resScale when stroking the path.
// The resulting filled path is incorrect. Note that stroking with a smaller resScale works fine.
DEF_SIMPLE_GM(bug12866, canvas, 128, 64) {
    SkPaint strokePaint;
    strokePaint.setAntiAlias(true);
    strokePaint.setStyle(SkPaint::kStroke_Style);
    strokePaint.setStrokeWidth(3);

    SkPaint fillPaint;
    fillPaint.setAntiAlias(true);

    SkPath strokePath = get_path();
    SkPath fillPath;
    skpathutils::FillPathWithPaint(strokePath, strokePaint, &fillPath, nullptr, 1200.0f);

    SkRect strokeBounds = strokePath.getBounds();
    SkRect fillBounds = fillPath.getBounds();

    // Draw the stroked path. This (internally) uses a resScale of 1.0, and looks good.
    canvas->save();
    canvas->translate(10 - strokeBounds.fLeft, 10 - strokeBounds.fTop);
    canvas->drawPath(strokePath, strokePaint);
    canvas->restore();

    // With a perspective CTM, it's possible for resScale to become large. Draw the filled
    // path produced by the stroker in that situation, which ends up being incorrect.
    canvas->save();
    canvas->translate(74 - fillBounds.fLeft, 10 - fillBounds.fTop);
    canvas->drawPath(fillPath, fillPaint);
    canvas->restore();
}
