/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBorder.h"

SkBorder::SkBorder(SkPaint& p, SkScalar width, BorderStyle style)
    : fFlags(kOnePaint_Flag) {
    fPaints[0] = p;

    for (int i = 0; i < 4; ++i) {
        fWidths[i] = width;
        fStyles[i] = style;
    }
}

SkBorder::SkBorder(const SkPaint paints[4],
                   const SkScalar widths[4],
                   const BorderStyle styles[4])
    : fFlags(0) {
    for (int i = 0; i < 4; ++i) {
        fPaints[i] = paints[i];
    }

    memcpy(fWidths, widths, sizeof(fWidths));
    memcpy(fStyles, styles, sizeof(fStyles));
}
