/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGColor.h"
#include "SkSGDraw.h"
#include "SkSGRect.h"

#include "gm.h"

DEF_SIMPLE_GM(SG, canvas, 256, 256) {

    auto rect = sksg::Rect::Make();
    rect->set_l(100);
    rect->set_t(100);
    rect->set_r(300);
    rect->set_b(200);

    auto color = sksg::Color::Make(SK_ColorGREEN);

    auto drawRect = sksg::Draw::Make(rect, color);

    drawRect->render(canvas);
}
