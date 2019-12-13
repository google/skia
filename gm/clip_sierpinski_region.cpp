/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRegion.h"

static constexpr int kSize = 3*3*3*3*3;
static constexpr int kTrans = 10;

DEF_SIMPLE_GM(clip_sierpinski_region, canvas, 2*kTrans + kSize, 2*kTrans + kSize) {
    SkRegion region;
    static constexpr int kSteps = 4;
    int n = 1;
    SkScalar s = kSize/3.f;
    for (int i = 0; i < kSteps; ++i, n*=3, s/=3.f) {
        for (int x = 0; x < n; ++x) {
            for (int y = 0; y < n; ++y) {
                region.op(SkIRect::MakeXYWH((3*x + 1)*s, (3*y + 1)*s, s, s), SkRegion::kUnion_Op);
            }
        }
    }
    // Test that a save layer with an offset works as expected.
    region.translate(kTrans, kTrans);
    canvas->saveLayer(SkRect::MakeXYWH(kTrans, kTrans, 1000, 1000), nullptr);
    // Make sure the clip call ignores the CTM.
    canvas->rotate(25.f, 50.f, 50.f);
    canvas->clipRegion(region);
    SkPaint red;
    red.setColor(SK_ColorRED);
    canvas->drawPaint(red);
    canvas->restore();
}
