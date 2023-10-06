/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkShader.h"
#include "include/core/SkTileMode.h"
#include "tools/DecodeUtils.h"
#include "tools/Resources.h"

DEF_SIMPLE_GM(coordclampshader, canvas, 1074, 795) {
    auto image = ToolUtils::GetResourceAsImage("images/mandrill_256.png");
    if (!image) {
        return;
    }
    // The mandrill_512 image has a bottom row of mostly black pixels. Remove it.
    image = image->makeSubset(nullptr, SkIRect::MakeWH(image->width(), image->height() - 1));
    image = image->withDefaultMipmaps();

    auto imageShader = image->makeShader(SkFilterMode::kLinear);

    SkPaint paint;

    auto drawRect = SkRect::Make(image->dimensions());

    auto rotate = SkMatrix::RotateDeg(45.f, drawRect.center());

    auto clampRect = drawRect.makeInset(20, 40);

    canvas->translate(10, 10);

    auto shader = SkShaders::CoordClamp(imageShader, clampRect);
    paint.setShader(std::move(shader));
    canvas->drawRect(drawRect, paint);

    canvas->save();
    canvas->translate(image->width(), 0);
    shader = SkShaders::CoordClamp(imageShader->makeWithLocalMatrix(rotate), clampRect);
    paint.setShader(std::move(shader));
    canvas->drawRect(drawRect, paint);
    canvas->restore();

    canvas->save();
    canvas->translate(0, image->height());
    shader = SkShaders::CoordClamp(imageShader, clampRect)->makeWithLocalMatrix(rotate);
    paint.setShader(std::move(shader));
    canvas->drawRect(drawRect, paint);
    canvas->restore();

    canvas->save();
    canvas->translate(image->width(), image->height());
    shader = SkShaders::CoordClamp(imageShader->makeWithLocalMatrix(rotate), clampRect)
                     ->makeWithLocalMatrix(rotate);
    paint.setShader(std::move(shader));
    canvas->drawRect(drawRect, paint);
    canvas->restore();

    canvas->translate(0, 2 * image->height() + 10);

    static const SkSamplingOptions kSamplers[] = {
            SkSamplingOptions{SkFilterMode::kNearest},
            SkSamplingOptions{SkFilterMode::kLinear},
            SkSamplingOptions{SkFilterMode::kLinear, SkMipmapMode::kLinear},
            SkSamplingOptions::Aniso(16)
    };

    for (const auto& sampler : kSamplers) {
        imageShader = image->makeShader(SkTileMode::kMirror,
                                        SkTileMode::kMirror,
                                        sampler,
                                        SkMatrix::Scale(0.3f, 1.0));

        shader = SkShaders::CoordClamp(imageShader, clampRect);
        paint.setShader(std::move(shader));
        canvas->drawRect(drawRect, paint);

        canvas->translate(image->width() + 10, 0);
    }
}
