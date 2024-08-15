/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"

DEF_SIMPLE_GM(luminosity_overflow, canvas, 256, 256) {
    // As reported in b/359049360:
    // The luminosity blend mode includes a division that can behave badly (black results) when
    // drawing low-alpha content over bright backgrounds. This GM reproduced the effect on several
    // GPUs. Proper rendering should be various near-white colors, with no black boxes.
    constexpr int kRGBs[] = {243, 247, 251, 255};
    canvas->save();
    for (int r : kRGBs) {
        for (int g : kRGBs) {
            for (int b : kRGBs) {
                SkPaint p;
                p.setColor(SkColorSetARGB(255, r, g, b));
                canvas->drawRect(SkRect::MakeWH(4, 256), p);
                canvas->translate(4, 0);
            }
        }
    }
    canvas->restore();

    for (int a = 1; a <= 16; ++a) {
        SkPaint p;
        p.setColor(SkColorSetARGB(a, 255, 255, 255));
        p.setBlendMode(SkBlendMode::kLuminosity);
        canvas->drawRect(SkRect::MakeWH(256, 16), p);
        canvas->translate(0, 16);
    }
}
