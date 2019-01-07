/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkFont.h"
#include "SkPath.h"

// This GM shows off a flaw in delta-based rasterizers (DAA, CCPR, etc.).
// See also the bottom of dashing4 and skia:6886.

static const int K = 50;

DEF_SIMPLE_GM(daa, canvas, K+350, K) {
    SkPaint paint;
    paint.setAntiAlias(true);

    paint.setColor(SK_ColorBLACK);
    canvas->drawString("Should be a green square with no red showing through.",
                       K*1.5f, K/2, SkFont(), paint);

    paint.setColor(SK_ColorRED);
    canvas->drawRect({0,0,K,K}, paint);

    SkPath path;
    SkPoint tri1[] = {{0,0},{K,K},{0,K},{0,0}};
    SkPoint tri2[] = {{0,0},{K,K},{K,0},{0,0}};
    path.addPoly(tri1, SK_ARRAY_COUNT(tri1), false);
    path.addPoly(tri2, SK_ARRAY_COUNT(tri2), false);

    paint.setColor(SK_ColorGREEN);
    canvas->drawPath(path, paint);
}
