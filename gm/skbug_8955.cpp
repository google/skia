/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkTextBlob.h"

DEF_SIMPLE_GM(skbug_8955, canvas, 100, 100) {
    SkPaint p;
    SkFont font;
    font.setSize(50);
    auto blob = SkTextBlob::MakeFromText("+", 1, font);

    // This bug only appeared when drawing the same text blob. We would generate no glyphs on the
    // first draw, and fail to mark the blob as having any bitmap runs. That would prevent us from
    // re-generating the blob on the second draw, even though the matrix had been restored.
    canvas->save();
    canvas->scale(0, 0);
    canvas->drawTextBlob(blob, 30, 60, p);
    canvas->restore();
    canvas->drawTextBlob(blob, 30, 60, p);
}
