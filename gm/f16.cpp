/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "Resources.h"
#include "SkCodec.h"

static SkImageInfo make_f16_info(SkISize size) {
    return SkImageInfo::Make(size.width(),
                             size.height(),
                             kRGBA_F16_SkColorType,
                             kPremul_SkAlphaType,
                             SkColorSpace::MakeSRGBLinear());
}

DEF_SIMPLE_GM(f16, canvas, 256, 256) {
    std::unique_ptr<SkCodec> codec(SkCodec::NewFromData(GetResourceAsData("mandrill_256.png")));
    if (!codec) {
        return;
    }
    SkBitmap bm;
    bm.allocPixels(make_f16_info(codec->getInfo().dimensions()));
    SkCodec::Result r = codec->getPixels(bm.info(), bm.getPixels(), bm.rowBytes());
    if (SkCodec::kSuccess != r) {
        SkDebugf("\n%s: SkCodec::Result = %d\n", __func__, r);
        canvas->clear(SK_ColorRED);
        return;
    }
    canvas->drawBitmap(bm, 0, 0);
}
