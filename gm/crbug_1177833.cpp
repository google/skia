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

// Bad quads dumped from SkiaRenderer in crbug.com/1178833. These should all draw as really thin
// lines.
DEF_SIMPLE_GM(crbug_1177833, canvas, 400, 400) {
    canvas->clear(SK_ColorBLACK);
    canvas->translate(-700, -700);
    // This quad had two issues. The inset collapsed the inner 2D projected quad to a point but
    // didn't enable enough degrees of freedom to adjust the 4 3D points to project to that point.
    // Also, the outset produced a 2D projected point far away from the original quad but the
    // shader was not checking the geometric subset and so pixels far away from the projection of
    // the quad would have positive coverage.
    {
        canvas->save();
        canvas->concat(SkMatrix::MakeAll(SkBits2Float(0xbf79250e), SkBits2Float(0x3e9da860), SkBits2Float(0x44914c8a),
                                         SkBits2Float(0xbf982962), SkBits2Float(0xbf280002), SkBits2Float(0x44c3116e),
                                         SkBits2Float(0xba9bfe62), SkBits2Float(0x39d10455), SkBits2Float(0x3fc9b377)));
        SkRect rect = {SkBits2Float(0x00000000),
                       SkBits2Float(0x00000000),
                       SkBits2Float(0x40a00000),
                       SkBits2Float(0x43560000)};
        SkPoint clip[4] = {{SkBits2Float(0x409fff57), SkBits2Float(0x40c86a18)},
                           {SkBits2Float(0x409fff57), SkBits2Float(0x4314dc8c)},
                           {SkBits2Float(0x407f6b0d), SkBits2Float(0x43157fff)},
                           {SkBits2Float(0x4040859c), SkBits2Float(0x43140374)}};
        SkCanvas::QuadAAFlags aaFlags = static_cast<SkCanvas::QuadAAFlags>(0x00000002);
        SkColor4f color = {SkBits2Float(0x3f6eeef0),
                           SkBits2Float(0x3f6eeef0),
                           SkBits2Float(0x3f6eeef0),
                           SkBits2Float(0x3f800000)};
        SkBlendMode mode = static_cast<SkBlendMode>(0x00000003);
        canvas->experimental_DrawEdgeAAQuad(rect, clip, aaFlags, color, mode);
        canvas->restore();
    }
    // This quad also exposed the inset collapse to a point without enough degrees of freedom issue.
    canvas->save();
    canvas->translate(-300, 0);
    {
        canvas->save();
        canvas->concat(SkMatrix::MakeAll(SkBits2Float(0x3f54dd8a), SkBits2Float(0xbf9096a4), SkBits2Float(0x447eae34),
                                         SkBits2Float(0x3f3f6905), SkBits2Float(0xbe5208ba), SkBits2Float(0x4418118b),
                                         SkBits2Float(0x3aa134a1), SkBits2Float(0xb93ef249), SkBits2Float(0x3f580bd4)));
        SkRect rect = {SkBits2Float(0x00000000),
                       SkBits2Float(0x00000000),
                       SkBits2Float(0x40a00000),
                       SkBits2Float(0x43560000)};
        SkPoint clip[4] = {{SkBits2Float(0x40a0000e), SkBits2Float(0x40c86b5a)},
                           {SkBits2Float(0x40a0001e), SkBits2Float(0x4314dd5f)},
                           {SkBits2Float(0x407f76eb), SkBits2Float(0x431580c2)},
                           {SkBits2Float(0x404092e7), SkBits2Float(0x43140445)}};
        SkCanvas::QuadAAFlags aaFlags = static_cast<SkCanvas::QuadAAFlags>(0x00000002);
        SkColor4f color = {SkBits2Float(0x3f6eeef0),
                           SkBits2Float(0x3f6eeef0),
                           SkBits2Float(0x3f6eeef0),
                           SkBits2Float(0x3f800000)};
        SkBlendMode mode = static_cast<SkBlendMode>(0x00000003);
        canvas->experimental_DrawEdgeAAQuad(rect, clip, aaFlags, color, mode);
        canvas->restore();
    }
    canvas->restore();
    // This quad exposed a similar issue to the point issue above, but when collapsing to a
    // triangle. When a 2D quad edge collapsed from insetting we'd replace it with a point off of
    // its adjacent edges. We need to ensure the code that moves the 3D point that projects to
    // the 2D point has 2 degrees of freedom so it can find the correct 3D point.
    {
        canvas->save();
        canvas->concat(SkMatrix::MakeAll(SkBits2Float(0x3f54b255), SkBits2Float(0x3eb5a94d), SkBits2Float(0x443d7419),
                                         SkBits2Float(0x3f885d66), SkBits2Float(0x3f5a6b9c), SkBits2Float(0x443c7334),
                                         SkBits2Float(0x3aa95ea5), SkBits2Float(0xb8a1391e), SkBits2Float(0x3f84dde5)));
        SkRect rect = {SkBits2Float(0x00000000),
                       SkBits2Float(0x00000000),
                       SkBits2Float(0x40a00000),
                       SkBits2Float(0x43100000)};
        SkPoint clip[4] = {{SkBits2Float(0x405a654c), SkBits2Float(0x42e8c790)},
                           {SkBits2Float(0x3728c61b), SkBits2Float(0x42e7df31)},
                           {SkBits2Float(0xb678ecc5), SkBits2Float(0x412db4e0)},
                           {SkBits2Float(0x4024b2ad), SkBits2Float(0x413ab3ed)}};
        SkCanvas::QuadAAFlags aaFlags = static_cast<SkCanvas::QuadAAFlags>(0x00000004);
        SkColor4f color = {SkBits2Float(0x3f800000),
                           SkBits2Float(0x3f800000),
                           SkBits2Float(0x3f800000),
                           SkBits2Float(0x3f800000)};
        SkBlendMode mode = static_cast<SkBlendMode>(0x00000003);
        canvas->experimental_DrawEdgeAAQuad(rect, clip, aaFlags, color, mode);
        canvas->restore();
    }
}

