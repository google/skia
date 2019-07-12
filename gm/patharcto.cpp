/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPath.h"
#include "include/core/SkPaint.h"

// crbug.com/982968
// Intended to draw a curvy triangle.
// With the bug, we extend the left side of the triangle with lines, a bit like a flag pole.
// The bug/fix is related to precision of the calculation. The original impl used all floats,
// but the very shallow angle causes our sin/cos and other calcs to lose too many bits.
// The fix was to use doubles for this part of the calc in SkPath::arcTo().
//
DEF_SIMPLE_GM(shallow_angle_path_arcto, canvas, 300, 300) {
    SkPath path;
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);

    path.moveTo(313.44189096331155f, 106.6009423589212f);
    path.arcTo(284.3113082008462f, 207.1407719157063f,
               255.15053777129728f, 307.6718505416374f,
               697212.0011054524f);
    path.lineTo(255.15053777129728f, 307.6718505416374f);
    path.arcTo(340.4737465981018f, 252.6907319346971f,
               433.54333477716153f, 212.18116363345337f,
               1251.2484277907251f);
    path.lineTo(433.54333477716153f, 212.18116363345337f);
    path.arcTo(350.19513833839466f, 185.89280014838369f,
               313.44189096331155f, 106.6009423589212f,
               198.03116885327813f);

    canvas->translate(-200, -50);
    canvas->drawPath(path, paint);
};
