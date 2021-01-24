/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"

static sk_sp<SkImage> make_bm() {
    SkBitmap bm;
    bm.allocN32Pixels(60, 60);
    bm.eraseColor(0);

    SkCanvas canvas(bm);
    SkPaint paint;
    canvas.drawPath(SkPath::Polygon({{6,6}, {6,54}, {30,54}}, false), paint);

    paint.setStyle(SkPaint::kStroke_Style);
    canvas.drawRect(SkRect::MakeLTRB(0.5f, 0.5f, 59.5f, 59.5f), paint);
    return bm.asImage();
}

// This creates a close, but imperfect concatenation of
//      scaling the image up by its dst-rect
//      scaling the image down by the matrix' scale
//  The bug was that for cases like this, we were incorrectly trying to take a
//  fast-path in the bitmapshader, but ended up drawing the last col of pixels
//  twice. The fix resulted in (a) not taking the fast-path, but (b) drawing
//  the image correctly.
//
DEF_SIMPLE_GM(bitmaprecttest, canvas, 320, 240) {
    auto image = make_bm();

    canvas->drawImage(image, 150, 45);

    SkScalar scale = 0.472560018f;
    canvas->save();
    canvas->scale(scale, scale);
    canvas->drawImageRect(image, SkRect::MakeXYWH(100, 100, 128, 128), SkSamplingOptions());
    canvas->restore();

    canvas->scale(-1, 1);
    canvas->drawImage(image, -310, 45);
}
