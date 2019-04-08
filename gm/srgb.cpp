/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "SkCanvas.h"
#include "SkColorFilter.h"
#include "SkImage.h"
#include "ToolUtils.h"
#include "gm.h"

DEF_SIMPLE_GM(srgb_colorfilter, canvas, 512, 256*3) {
    auto img = GetResourceAsImage("images/mandrill_256.png");

    const float array[] = {
        1, 0, 0, 0, 0,
        0, 1, 0, 0, 0,
        0, 0, 1, 0, 0,
        -1, 0, 0, 1, 0,
    };
    auto cf0 = SkColorFilters::MatrixRowMajor255(array);
    auto cf1 = SkColorFilters::LinearToSRGBGamma();
    auto cf2 = SkColorFilters::SRGBToLinearGamma();

    SkPaint p;
    p.setColorFilter(cf0);
    canvas->drawImage(img, 0, 0, nullptr);
    canvas->drawImage(img, 256, 0, &p);

    p.setColorFilter(cf1);
    canvas->drawImage(img, 0, 256, &p);
    p.setColorFilter(cf1->makeComposed(cf0));
    canvas->drawImage(img, 256, 256, &p);

    p.setColorFilter(cf2);
    canvas->drawImage(img, 0, 512, &p);
    p.setColorFilter(cf2->makeComposed(cf0));
    canvas->drawImage(img, 256, 512, &p);
}
