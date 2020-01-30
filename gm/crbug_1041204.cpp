/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkMatrix.h"

DEF_SIMPLE_GM(crbug_10141204, canvas, 512, 512) {
    // While the coordinates are giant and the transform is not axis-aligned, this should
    // fill the screen left side with solid blue. This has an extra zoom factor compared to the
    // canvas JS in order to more visibly highlight the numerical issues that caused the bug.
    // (The original transform would have completely filled the screen with solid blue, so the bug
    // manifested as an improper discard on occasion. With the new scale factor, the bug manifests
    // as either an improper fullscreen clear or an improper discard, instead).
    SkScalar extraZoom = exp(-2.3f);
    canvas->scale(extraZoom, extraZoom);
    canvas->scale(2.f, 2.f);
    canvas->concat(SkMatrix::MakeAll(
            -0.0005550860255665798f, -0.0030798374421905717f, -0.014111959825129805f,
            -0.07569627776417084f, 232.00000000000017f, 39.999999999999936f,
            0.f, 0.f, 1.f));
    canvas->translate(-3040103.0493857153f, 337502.1103282161f);
    canvas->scale(9783.93962050256f, -9783.93962050256f);

    SkPaint paint;
    paint.setColor(SK_ColorBLUE);
    paint.setAntiAlias(true);
    canvas->drawRect(SkRect::MakeWH(512, 512), paint);
}
