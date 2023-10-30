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
#include "include/effects/SkGradientShader.h"
#include "tools/fonts/FontToolUtils.h"

DEF_SIMPLE_GM(crbug_1073670, canvas, 250, 250) {
    SkPoint pts[] = {{0, 0}, {0, 250}};
    SkColor colors[] = {0xFFFF0000, 0xFF0000FF};
    auto sh = SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kClamp);
    SkPaint p;
    p.setShader(sh);

    SkFont f = ToolUtils::DefaultPortableFont();
    f.setSize(325);
    f.setEdging(SkFont::Edging::kAntiAlias);

    canvas->drawString("Gradient", 10, 250, f, p);
}
