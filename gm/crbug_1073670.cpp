/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/effects/SkGradient.h"
#include "tools/fonts/FontToolUtils.h"

DEF_SIMPLE_GM(crbug_1073670, canvas, 250, 250) {
    const SkPoint pts[] = {{0, 0}, {0, 250}};
    const SkColor4f colors[] = {{1,0,0,1}, {0,0,1,1}};
    auto sh = SkShaders::LinearGradient(pts, {{colors, {}, SkTileMode::kClamp}, {}});
    SkPaint p;
    p.setShader(sh);

    SkFont f = ToolUtils::DefaultPortableFont();
    f.setSize(325);
    f.setEdging(SkFont::Edging::kAntiAlias);

    canvas->drawString("Gradient", 10, 250, f, p);
}
