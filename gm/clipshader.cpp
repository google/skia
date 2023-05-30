/*
* Copyright 2011 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkGradientShader.h"
#include "tools/Resources.h"


// This tests using clip shader and then changing the canvas matrix before drawing. It also verifies
// that we don't incorrectly disable linear filtering of a clip image shader.
DEF_SIMPLE_GM(clipshadermatrix, canvas, 145, 128) {
    auto clipSurface = SkSurfaces::Raster(SkImageInfo::MakeA8({70, 60}));
    // Hard edged oval clip
    clipSurface->getCanvas()->drawOval(SkRect::MakeXYWH(0, 10, 64, 44), SkPaint{});
    auto clipShader = clipSurface->makeImageSnapshot()->makeShader(
            SkTileMode::kDecal, SkTileMode::kDecal, SkFilterMode::kLinear);

    canvas->translate(5, 0);
    for (auto tx : {0.f, 68.5f}) {
        for (auto ty : {0.f, 66.5f}) {
            canvas->save();

            canvas->translate(tx, ty);
            canvas->clipShader(clipShader);
            canvas->translate(-tx, -ty);

            SkMatrix m;
            m.setSkew(0.03f, 0.f);
            m.setPerspY( 0.0007f);
            m.setPerspX(-0.002f);
            m.setScaleX(1.2f); m.setScaleY(0.8f);
            m.preRotate(30.f);
            canvas->concat(m);

            SkPoint center = {64, 64};
            SkAssertResult(m.invert(&m));
            center = m.mapPoint(center);
            SkColor colors[] {SK_ColorYELLOW,  SK_ColorGREEN, SK_ColorBLUE,
                              SK_ColorMAGENTA, SK_ColorCYAN , SK_ColorYELLOW};
            auto gradient = SkGradientShader::MakeRadial(
                    center,
                    /*radius=*/32.f,
                    colors,
                    /*pos=*/nullptr,
                    std::size(colors),
                    SkTileMode::kMirror,
                    /*flags=*/0,
                    /*localMatrix=*/nullptr);

            SkPaint paint;
            paint.setShader(std::move(gradient));
            canvas->drawPaint(paint);

            canvas->restore();
        }
    }
}
