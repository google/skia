/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"

DEF_SIMPLE_GM(bicubic, canvas, 300, 64) {
    canvas->clear(SK_ColorBLACK);

    auto make_img = []() {
        auto surf = SkSurface::MakeRasterN32Premul(7, 7);
        surf->getCanvas()->drawColor(SK_ColorBLACK);

        SkPaint paint;
        paint.setColor(SK_ColorWHITE);
        surf->getCanvas()->drawLine(3.5f, 0, 3.5f, 8, paint);
        return surf->makeImageSnapshot();
    };

    auto img = make_img();

    canvas->scale(40, 8);
    for (auto q : {kNone_SkFilterQuality, kLow_SkFilterQuality, kHigh_SkFilterQuality}) {
        SkPaint p;
        p.setFilterQuality(q);
        canvas->drawImage(img, 0, 0, &p);
        canvas->translate(0, img->height() + 1.0f);
    }

    SkRect r = SkRect::MakeIWH(img->width(), img->height());
    SkM44 mx = SkImage::GetWeights(SkImage::FilterWeights::kCentripetalCatmulRom);
    SkPaint paint;
    paint.setShader(img->makeShader(SkTileMode::kClamp, SkTileMode::kClamp, mx));
    canvas->drawRect(r, paint);
}
