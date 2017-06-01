/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"

#include "Resources.h"
#include "SkBlendModePriv.h"
#include "SkGradientShader.h"
#include "SkPM4fPriv.h"

DEF_SIMPLE_GM(gamma, canvas, 850, 200) {
    SkPaint p;
    p.setStyle(SkPaint::kStroke_Style);
    float x = 10, y = 10;
    for (int i = 0; i < 2000; ++i) {
        canvas->drawCircle(x, y, 3, p);
        x += 10;
        if (x > 1000) {
            x = 10;
            y += 10;
        }
    }
}
