/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "skia.h"

static void draw(SkCanvas* canvas);
DEF_SIMPLE_GM(fiddle, canvas, 256, 256) { draw(canvas); }

// Paste your fiddle.skia.org code over this stub.
void draw(SkCanvas* canvas) {
    // SkMatrix::MakeAll(
    //    1.633362e+00f, 7.881372e-02f, 1.592217e+03f,
    //    9.125900e-01f, 1.348160e-01f, 2.890398e+02f,
    //    1.231794e-03f, 9.033346e-04f, 6.835773e-01f);
    SkMatrix ctm = SkMatrix::MakeAll(
            SkBits2Float(0x3FCC7F75), SkBits2Float(0x3D5784FC), SkBits2Float(0x44C48C99),
            SkBits2Float(0x3F699F7F), SkBits2Float(0x3E0A0D37), SkBits2Float(0x43908518),
            SkBits2Float(0x3AA17423), SkBits2Float(0x3A6CCDC3), SkBits2Float(0x3F2EFEEC));

    // {{7.246903e-01f, 5.109999e+02f}, {1.397926e-04f, 5.110000e+02f},
    //  {4.398279e-05f, 4.100005e+01f}, {7.246332e-01f, 4.100002e+01f}};
    SkPoint pts[4] = {{SkBits2Float(0x3F39778B), SkBits2Float(0x43FF7FFC)},
                      {SkBits2Float(0x0), SkBits2Float(0x43FF7FFA)},
                      {SkBits2Float(0xB83B055E), SkBits2Float(0x42500003)},
                      {SkBits2Float(0x3F39776F), SkBits2Float(0x4250000D)}};

    SkPath quadPath = SkPath::Polygon(pts, 4, true);
    quadPath.moveTo(pts[0]);
    quadPath.lineTo(pts[1]);

    canvas->clear(SK_ColorWHITE);
    canvas->setMatrix(ctm);

    // green (non-AA) should be fine since it'll skip insetting
    canvas->experimental_DrawEdgeAAQuad(quadPath.getBounds(), pts, SkCanvas::kNone_QuadAAFlags,
                                        SK_ColorGREEN, SkBlendMode::kSrcOver);
    // blue (aa) should be bad since it'll become unstable when insetting
    canvas->experimental_DrawEdgeAAQuad(quadPath.getBounds(), pts,
                                        (SkCanvas::QuadAAFlags) (SkCanvas::kBottom_QuadAAFlag | SkCanvas::kRight_QuadAAFlag),
                                        {0.f, 0.f, 1.f, 0.5f}, SkBlendMode::kSrcOver);

    SkPaint outline;
    outline.setAntiAlias(true);
    outline.setStyle(SkPaint::kStroke_Style);
    //canvas->drawPath(quadPath, outline);
}
