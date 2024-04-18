/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRect.h"
#include "src/base/SkFloatBits.h"

// This quad would, depending on which aa flags are used, would degenerate when inset. We'd replace
// and duplicate some of the inset points to make a triangle. However, one of the triangle points
// would be far outside the original quad.
DEF_SIMPLE_GM(crbug_1167277, canvas, 230, 320) {
    canvas->translate(-1250, -900);
    // Matrix, clip, and quad values taken from Chrome repro scenario.
    SkMatrix ctm = SkMatrix::MakeAll(
            SkBits2Float(0xbf8fcfae), SkBits2Float(0xbeae25ee), SkBits2Float(0x449ca6db),
            SkBits2Float(0x3c9dc40f), SkBits2Float(0xbf950e35), SkBits2Float(0x4487da43),
            SkBits2Float(0xb8d4d6bc), SkBits2Float(0xb92fbb29), SkBits2Float(0x3f6f605c));
    SkRect rect = {SkBits2Float(0x00000000), SkBits2Float(0x00000000),
                   SkBits2Float(0x41880000), SkBits2Float(0x43440000)};
    SkPoint clip[4] = {{SkBits2Float(0x3ef434a2), SkBits2Float(0x43440004)},
                       {SkBits2Float(0x00000000), SkBits2Float(0x43440009)},
                       {SkBits2Float(0x38ef605d), SkBits2Float(0x38ef605d)},
                       {SkBits2Float(0x3ef436e3), SkBits2Float(0x396f5d30)}};
    SkColor color = SK_ColorGREEN;
    for (int flags = 0; flags < static_cast<int>(SkCanvas::kAll_QuadAAFlags); ++flags) {
        SkCanvas::QuadAAFlags aaFlags = static_cast<SkCanvas::QuadAAFlags>(flags);
        canvas->save();
        canvas->concat(ctm);
        canvas->experimental_DrawEdgeAAQuad(rect, clip, aaFlags, color, SkBlendMode::kSrcOver);
        canvas->restore();
        canvas->translate(5, 0);
        SkColor rgb = color & 0x00FFFFFF;
        color = 0xFF000000 | (rgb << 4) | (rgb >> 20);
    }
}

