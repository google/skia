/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkRefCnt.h"
#include "tools/Resources.h"

DEF_SIMPLE_GM(makeRasterImage, canvas, 128,128) {
    if (auto img = GetResourceAsImage("images/color_wheel.png")) {
        canvas->drawImage(img->makeRasterImage(), 0,0);
    }
}
