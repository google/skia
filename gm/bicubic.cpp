/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"

DEF_SIMPLE_GM(bicubic, canvas, 300, 320) {
    canvas->clear(SK_ColorBLACK);

    const SkSamplingOptions gSamplings[] = {
        SkSamplingOptions(SkFilterMode::kNearest),
        SkSamplingOptions(SkFilterMode::kLinear),
        SkSamplingOptions(SkCubicResampler::Mitchell()),
    };

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
    for (const auto& s : gSamplings) {
        canvas->drawImage(img, 0, 0, s, nullptr);
        canvas->translate(0, img->height() + 1.0f);
    }

    const SkRect r = SkRect::MakeIWH(img->width(), img->height());
    SkPaint paint;

    SkImage::CubicResampler cubics[] = {
        SkCubicResampler::CatmullRom(),
        SkCubicResampler::Mitchell(),
    };
    for (auto c : cubics) {
        paint.setShader(img->makeShader(SkTileMode::kClamp, SkTileMode::kClamp,
                                        SkSamplingOptions(c)));
        canvas->drawRect(r, paint);
        canvas->translate(0, img->height() + 1.0f);
    }
}
