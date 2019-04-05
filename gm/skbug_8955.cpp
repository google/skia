/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "SkFont.h"
#include "SkPaint.h"
#include "SkTextBlob.h"

DEF_SIMPLE_GM(skbug_8955, canvas, 100, 100) {
    SkPaint p;
    SkFont font;
    font.setSize(50);
    auto blob = SkTextBlob::MakeFromText("+", 1, font);

    canvas->save();
    canvas->scale(0, 0);
    canvas->drawTextBlob(blob, 30, 60, p);
    canvas->restore();
    canvas->drawTextBlob(blob, 30, 60, p);
}
