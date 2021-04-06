/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRefCnt.h"
#include "tools/Resources.h"

DEF_SIMPLE_GM(srgb_colorfilter, canvas, 512, 256*3) {
    auto img = GetResourceAsImage("images/mandrill_256.png");

    const float array[] = {
        1, 0, 0, 0, 0,
        0, 1, 0, 0, 0,
        0, 0, 1, 0, 0,
        -1, 0, 0, 1, 0,
    };
    auto cf0 = SkColorFilters::Matrix(array);
    auto cf1 = SkColorFilters::LinearToSRGBGamma();
    auto cf2 = SkColorFilters::SRGBToLinearGamma();

    SkSamplingOptions sampling;
    SkPaint p;
    p.setColorFilter(cf0);
    canvas->drawImage(img, 0, 0);
    canvas->drawImage(img, 256, 0, sampling, &p);

    p.setColorFilter(cf1);
    canvas->drawImage(img, 0, 256, sampling, &p);
    p.setColorFilter(cf1->makeComposed(cf0));
    canvas->drawImage(img, 256, 256, sampling, &p);

    p.setColorFilter(cf2);
    canvas->drawImage(img, 0, 512, sampling, &p);
    p.setColorFilter(cf2->makeComposed(cf0));
    canvas->drawImage(img, 256, 512, sampling, &p);
}

DEF_SIMPLE_GM(srgb_colortype, canvas, 260*2, 260) {
    auto draw = [canvas](SkColorType ct, sk_sp<SkColorSpace> cs) {
        uint32_t buf[] = {
            0xff0000ff, 0xff7f007f, 0xffff0000,
            0xff007f7f, 0xff3f3f7f, 0xff7f0000,
            0xff00ff00, 0xff007f00, 0xff000000,
        };
        SkPixmap pm = {
            SkImageInfo::Make(3,3, ct, kUnpremul_SkAlphaType, cs),
            buf,
            sizeof(buf)/3,
        };
        canvas->drawImageRect(SkImage::MakeRasterCopy(pm),
                              {2,2,258,258},
                              SkSamplingOptions{SkFilterMode::kLinear});
    };

    draw(kRGBA_8888_SkColorType,  SkColorSpace::MakeSRGB());
    canvas->translate(260,0);
    draw(kRGBA_8888_SkColorType,  SkColorSpace::MakeSRGBLinear());
    canvas->translate(260,0);
    draw(kSRGBA_8888_SkColorType, SkColorSpace::MakeSRGBLinear());
}
