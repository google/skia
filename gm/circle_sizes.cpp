/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"

// https://crbug.com/772953
DEF_SIMPLE_GM(circle_sizes, canvas, 128, 128) {
    SkPaint p;
    p.setAntiAlias(true);
    for (int i = 0; i < 16; ++i) {
        canvas->drawCircle({14.0f + 32.0f * (i % 4),
                            14.0f + 32.0f * (i / 4)}, i + 1.0f, p);
    }
}
