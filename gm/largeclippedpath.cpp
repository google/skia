/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPath.h"

constexpr int kSize = 1000;

// Makes sure GrPathInnerTriangulateOp uses correct stencil settings when there is a clip in the
// stencil buffer.
static void draw_clipped_flower(SkCanvas* canvas, SkPathFillType fillType) {
    canvas->clear(SK_ColorCYAN);
    SkPath clip;
    clip.setFillType(SkPathFillType::kWinding);
    constexpr static int kGridCount = 50;
    constexpr static float kCellSize = (float)kSize / kGridCount;
    for (int y = 0; y < kGridCount; ++y) {
        clip.addRect(0, y * kCellSize, kSize, (y + 1) * kCellSize, SkPathDirection(y & 1));
    }
    for (int x = 0; x < kGridCount; ++x) {
        clip.addRect(x * kCellSize, 0, (x + 1) * kCellSize, kSize, SkPathDirection(x & 1));
    }
    canvas->clipPath(clip);
    SkPath flower;
    flower.setFillType(fillType);
    flower.moveTo(1, 0);
    constexpr static int kNumPetals = 9;
    for (int i = 1; i <= kNumPetals; ++i) {
        float c = 2*SK_ScalarPI*(i - .5f) / kNumPetals;
        float theta = 2*SK_ScalarPI*i / kNumPetals;
        flower.quadTo(cosf(c)*2, sinf(c)*2, cosf(theta), sinf(theta));
    }
    flower.close();
    flower.addArc(SkRect::MakeLTRB(-.75f, -.75f, .75f, .75f), 0, 360);
    canvas->translate(kSize/2.f, kSize/2.f);
    canvas->scale(kSize/3.f, kSize/3.f);
    SkPaint p;
    p.setAntiAlias(true);
    p.setColor(SK_ColorMAGENTA);
    canvas->drawPath(flower, p);
}

DEF_SIMPLE_GM(largeclippedpath_winding, canvas, kSize, kSize) {
    draw_clipped_flower(canvas, SkPathFillType::kWinding);
}

DEF_SIMPLE_GM(largeclippedpath_evenodd, canvas, kSize, kSize) {
    draw_clipped_flower(canvas, SkPathFillType::kEvenOdd);
}
