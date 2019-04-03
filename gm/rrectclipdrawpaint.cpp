/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkPath.h"
#include "SkGradientShader.h"

// Exercises code in GrRenderTargetContext that attempts to replace a rrect clip/draw paint with
// draw rrect.
DEF_SIMPLE_GM(rrect_clip_draw_paint, canvas, 256, 256) {
    SkRRect rrect = SkRRect::MakeRectXY(SkRect::MakeXYWH(10.f, 10.f, 236.f, 236.f), 30.f, 40.f);

    SkPaint p;
    p.setColor(SK_ColorRED);

    SkMatrix zoomOut;
    zoomOut.setScale(0.7f, 0.7f, 128.f, 128.f);

    const SkRect layerRect = SkRect::MakeWH(256.f, 256.f);
    canvas->saveLayer(layerRect, nullptr);
    canvas->clipRRect(rrect, true);
    canvas->drawPaint(p);
    canvas->restore();

    canvas->concat(zoomOut);
    p.setColor(SK_ColorBLUE);
    canvas->saveLayer(layerRect, nullptr);
    canvas->clipRRect(rrect, false);
    canvas->drawPaint(p);
    canvas->restore();

    constexpr SkPoint kPts[] = {{0.f, 0.f}, {256.f, 256.f}};
    constexpr SkColor kColors1[] = {SK_ColorCYAN, SK_ColorGREEN};
    p.setShader(SkGradientShader::MakeLinear(kPts, kColors1, nullptr, 2, SkTileMode::kClamp));
    canvas->concat(zoomOut);
    canvas->saveLayer(layerRect, nullptr);
    canvas->clipRRect(rrect, true);
    canvas->drawPaint(p);
    canvas->restore();

    constexpr SkColor kColors2[] = {SK_ColorMAGENTA, SK_ColorGRAY};
    p.setShader(SkGradientShader::MakeRadial({128.f, 128.f}, 128.f, kColors2, nullptr, 2,
                                             SkTileMode::kClamp));
    canvas->concat(zoomOut);
    canvas->saveLayer(layerRect, nullptr);
    canvas->clipRRect(rrect, false);
    canvas->drawPaint(p);
    canvas->restore();
}
