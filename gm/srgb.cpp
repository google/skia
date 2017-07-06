/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkCanvas.h"
#include "SkImage.h"
#include "Resources.h"
#include "SkColorFilter.h"

DEF_SIMPLE_GM(srgb_colorfilter, canvas, 512, 256*3) {
    auto img = GetResourceAsImage("mandrill_256.png");

    const float array[] = {
        1, 0, 0, 0, 0,
        0, 1, 0, 0, 0,
        0, 0, 1, 0, 0,
        -1, 0, 0, 1, 0,
    };
    auto cf0 = SkColorFilter::MakeMatrixFilterRowMajor255(array);
    auto cf1 = SkColorFilter::MakeLinearToSRGBGamma();
    auto cf2 = SkColorFilter::MakeSRGBToLinearGamma();

    SkPaint p;
    p.setColorFilter(cf0);
    canvas->drawImage(img, 0, 0, nullptr);
    canvas->drawImage(img, 256, 0, &p);

    p.setColorFilter(cf1);
    canvas->drawImage(img, 0, 256, &p);
    p.setColorFilter(SkColorFilter::MakeComposeFilter(cf1, cf0));
    canvas->drawImage(img, 256, 256, &p);

    p.setColorFilter(cf2);
    canvas->drawImage(img, 0, 512, &p);
    p.setColorFilter(SkColorFilter::MakeComposeFilter(cf2, cf0));
    canvas->drawImage(img, 256, 512, &p);
}
