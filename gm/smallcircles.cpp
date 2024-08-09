/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"

static void draw_small_circle(SkCanvas* canvas, const SkPaint& p, const float radius) {
    SkRect oval = SkRect::MakeLTRB(radius * -1, radius * -1, radius, radius);
    constexpr bool kUseCenter = false;
    constexpr float kDegrees = 360;
    canvas->drawArc(oval, 0, kDegrees, kUseCenter, p);
}

DEF_SIMPLE_GM(smallcircles, canvas, 425, 425) {
    auto surface = canvas->makeSurface(SkImageInfo::MakeN32Premul(100, 100));
    if (!surface) {
      surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(100, 100));
    }
    SkCanvas* canv = surface->getCanvas();

    canv->translate(5, 5);
    SkPaint p;
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kFill_Style);

    // Draw circles
    for (int i = 0; i < 11; ++i) {
      canv->save();
      for (int j = 0; j < 11; ++j) {
        draw_small_circle(canv, p, 0.8f);
        canv->translate(5.1f, 0.f);
      }
      canv->restore();
      canv->translate(0.f, 5.1f);
    }

    // Scale up image of small circles
    canvas->scale(7, 7);
    auto img = surface->makeImageSnapshot();
    canvas->drawImage(img, 0, 0);
}
