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

// Outsetting of very thin, nearly line, quads for AA would go haywire and draw far outside of the
// quad. Artifacts seemed to occur more when coord values are large, hence the large GM size. At the
// time of GM creation this was fixed by dropping AA, which makes these quads practically never draw
// as it's very unlikely a pixel center would fall inside the geometry.
DEF_SIMPLE_GM(crbug_1174186, canvas, 1200, 1200) {
    auto m = SkMatrix::MakeAll(
            SkBits2Float(0x24480629), SkBits2Float(0xbf3555c2), SkBits2Float(0x4377d67b),
            SkBits2Float(0x23a61d51), SkBits2Float(0x3f34b400), SkBits2Float(0x4453f572),
            SkBits2Float(0x00000000), SkBits2Float(0x00000000), SkBits2Float(0x3f800000));

    SkPoint pts[] = {{SkBits2Float(0x3f7ffff2), SkBits2Float(0x43483d60)},
                     {SkBits2Float(0x00000000), SkBits2Float(0x43483d60)},
                     {SkBits2Float(0x00000000), SkBits2Float(0x4311a628)},
                     {SkBits2Float(0x3f800000), SkBits2Float(0x43130f8c)}};
    SkColor color = SK_ColorGREEN;
    canvas->translate(-500, 0);
    for (int i = 0; i < 10; ++i) {
        for (int flags = 0; flags < static_cast<int>(SkCanvas::kAll_QuadAAFlags); ++flags) {
            SkCanvas::QuadAAFlags aaFlags = static_cast<SkCanvas::QuadAAFlags>(flags);
            canvas->save();
            canvas->concat(m);
            canvas->experimental_DrawEdgeAAQuad(SkRect::MakeWH(1000, 1000), pts, aaFlags, color,
                                                SkBlendMode::kSrcOver);
            canvas->restore();
            canvas->translate(5.1f, 0);
            SkColor rgb = color & 0x00FFFFFF;
            color = 0xFF000000 | (rgb << 4) | (rgb >> 20);
        }
    }
}
