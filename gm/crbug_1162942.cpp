/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRect.h"

// This tests a scenario where when the 2D projection of a perspective quad is inset we degenerate
// the inset 2d geometry to a triangle because an inset vertex crosses the opposite edge. When we
// project back to 3D and try to move the original verts along the original edges so that they
// project to the 2D points. Whether an edge can be moved along depends on whether the adjacent edge
// at the vertex is AA. However, in the degenerate triangle case the 2D point may not fall along
// either of the edges. Thus, if we're constrained to moving along one edge and solve for X or Y the
// other value may go wildly away from projecting to the 2D value. The current approach is to force
// AA on at both edges that meet at a vertex whose inset point has been replaced by an off-edge
// point if either is AA originally. This gives us an additional vector to move along so that we can
// find a 3D point that projects to the 2D point in both X and Y.
DEF_SIMPLE_GM(crbug_1162942, canvas, 620, 200) {
    // Matrix and quad values taken from Chrome repro scenario.
    SkMatrix ctm = SkMatrix::MakeAll(
            SkBits2Float(0x3FCC7F75), SkBits2Float(0x3D5784FC), SkBits2Float(0x44C48C99),
            SkBits2Float(0x3F699F7F), SkBits2Float(0x3E0A0D37), SkBits2Float(0x43908518),
            SkBits2Float(0x3AA17423), SkBits2Float(0x3A6CCDC3), SkBits2Float(0x3F2EFEEC));
    ctm.postTranslate(-1500.f, -325.f);

    SkPoint pts[4] = {{SkBits2Float(0x3F39778B), SkBits2Float(0x43FF7FFC)},
                      {SkBits2Float(0x0), SkBits2Float(0x43FF7FFA)},
                      {SkBits2Float(0xB83B055E), SkBits2Float(0x42500003)},
                      {SkBits2Float(0x3F39776F), SkBits2Float(0x4250000D)}};
    SkRect bounds;
    bounds.setBounds(pts, 4);

    canvas->clear(SK_ColorWHITE);

    SkCanvas::QuadAAFlags flags[] = {
            (SkCanvas::QuadAAFlags) (SkCanvas::kTop_QuadAAFlag    | SkCanvas::kLeft_QuadAAFlag ),
            (SkCanvas::QuadAAFlags) (SkCanvas::kBottom_QuadAAFlag | SkCanvas::kRight_QuadAAFlag),
            (SkCanvas::QuadAAFlags) (SkCanvas::kBottom_QuadAAFlag),
            (SkCanvas::QuadAAFlags) (SkCanvas::kRight_QuadAAFlag),
            (SkCanvas::QuadAAFlags) (SkCanvas::kRight_QuadAAFlag  | SkCanvas::kLeft_QuadAAFlag),
            (SkCanvas::QuadAAFlags) (SkCanvas::kTop_QuadAAFlag    | SkCanvas::kBottom_QuadAAFlag),
    };

    SkColor color = SK_ColorGREEN;
    for (auto aaFlags : flags) {
        canvas->save();
        canvas->concat(ctm);
        canvas->experimental_DrawEdgeAAQuad(bounds, pts, aaFlags, color, SkBlendMode::kSrcOver);
        SkColor rgb = color & 0x00FFFFFF;
        color = 0xFF000000 | (rgb << 4) | (rgb >> 20);
        canvas->restore();
        canvas->translate(0, 25);
    }
}

