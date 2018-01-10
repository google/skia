/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "SkSurface.h"
#include "gm.h"

DEF_SIMPLE_GM(makeRasterImage, canvas, 128,128) {
    auto img = GetResourceAsImage("images/color_wheel.png");
    canvas->drawImage(img->makeRasterImage(), 0,0);
}
