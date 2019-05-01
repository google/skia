/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"

// The root cause of this bug was that when dual source blending was not supported and we made a
// copy of the destination to perform blending we would clip the copy bounds to the current clip.
// However, it is possible for anti-aliased that are fully contained by the clip in a geometric
// sense to actually draw outside the clip in pixel space because we don't consider aa bloat when
// determining if the draw is contained by the clip.
DEF_SIMPLE_GM(crbug_892988, canvas, 256, 256) {
    SkPaint paint1;
    paint1.setStyle(SkPaint::kStroke_Style);
    paint1.setStrokeWidth(1.f);
    paint1.setAntiAlias(true);
    canvas->drawRect(SkRect::MakeLTRB(11.5, 0.5, 245.5, 245.5), paint1);
    canvas->clipRect(SkRect::MakeLTRB(12, 1, 244, 244), true);
    SkPaint paint2;
    // Use src mode with a non-opaque color to produce a blend that can't be handled with
    // simple blend coefficients.
    paint2.setColor(0xF0FFFFFF);
    paint2.setBlendMode(SkBlendMode::kSrc);
    paint2.setAntiAlias(true);
    canvas->drawRect(SkRect::MakeLTRB(12, 1, 244, 244), paint2);
}
