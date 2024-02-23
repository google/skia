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

// This is another example of the same underlying bug (recursion limit in the stroker),
// but with cubics, rather than quads.
DEF_SIMPLE_GM(bug40810065, canvas, 256, 512) {
    canvas->scale(2.f, 2.f);

    SkPath path1;
    path1.moveTo(108.87f, 3.78f);
    path1.cubicTo(201.1f, -128.61f, 34.21f, 82.54f, 134.14f, 126.01f);
    SkPath path2;
    path2.moveTo(108.87f, 3.78f);
    path2.cubicTo(201.f, -128.61f, 34.21f, 82.54f, 134.14f, 126.f);

    SkPaint stroke;
    stroke.setColor(SK_ColorBLACK);
    stroke.setAntiAlias(true);
    stroke.setStyle(SkPaint::kStroke_Style);
    stroke.setStrokeWidth(1.f);
    stroke.setStrokeCap(SkPaint::kRound_Cap);

    canvas->save();
    canvas->translate(-75.f, 50.f);
    canvas->drawPath(path1, stroke);
    canvas->restore();

    canvas->save();
    canvas->translate(-20.f, 100.f);
    canvas->drawPath(path2, stroke);
    canvas->restore();
}

// Finally: A repro case that involves conics. This should draw NOTHING. When incorrect, it drew
// a large black rectangle over half of the slide.
DEF_SIMPLE_GM_BG(bug41422450, canvas, 863, 473, SK_ColorWHITE) {
    SkM44 mat{1, -0.00000139566271f, 0, -2321738,
              0.000113059919f, 0.0123444516f, 0, -353,
              0, 0, 1, 0,
              0, 0, 0, 1};
    canvas->concat(mat);

    SkPath strokePath;
    SkRect circle = SkRect::MakeLTRB(-3299135.5f, -12312541.0f, 9897407.0f, 884000.812f);
    strokePath.arcTo(circle, 59.9999962f, 59.9999962f, true);

    SkPaint strokePaint;
    strokePaint.setStyle(SkPaint::kStroke_Style);
    strokePaint.setStrokeWidth(2);
    canvas->drawPath(strokePath, strokePaint);
}
