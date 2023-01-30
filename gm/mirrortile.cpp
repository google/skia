/*
* Copyright 2022 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkShader.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTileMode.h"

/**
 * Tests image shader mirror tile mode with scale factors of 1 and -1, with nearest and linear
 * filtering, and with/without a half pixel offset between device and image space. The linear filter
 * should only have an effect when there is a half pixel offset. We test mirror tile mode in x and
 * in y separately.
 */
DEF_SIMPLE_GM_CAN_FAIL(mirror_tile, canvas, errorMsg, 140, 370) {
    // We don't run this test on the GPU because we're at the driver/hw's mercy for how this
    // is handled. We also don't test this on recording or vector canvases.
    if (SkPixmap unused; !canvas->peekPixels(&unused)) {
        *errorMsg = "Test only works with canvases backed by CPU pixels";
        return skiagm::DrawResult::kSkip;
    }

    uint32_t colors[] {0xFFFF0000, 0xFF00FF00, 0xFF0000FF};
    SkPixmap pmx(SkImageInfo::Make({std::size(colors), 1},
                                   kRGBA_8888_SkColorType,
                                   kPremul_SkAlphaType),
                 colors,
                 sizeof(colors));
    auto imgx = SkImage::MakeRasterCopy(pmx);

    SkPixmap pmy(SkImageInfo::Make({1, std::size(colors)},
                                   kRGBA_8888_SkColorType,
                                   kPremul_SkAlphaType),
                 colors,
                 sizeof(colors[0]));
    auto imgy = SkImage::MakeRasterCopy(pmy);

    // We draw offscreen and then zoom that up to make the result clear.
    auto surf = canvas->makeSurface(canvas->imageInfo().makeWH(80, 80));
    SkASSERT(surf);
    auto* c = surf->getCanvas();
    c->clear(SK_ColorWHITE);

    for (bool offset : {false, true}) {
        for (SkFilterMode fm : {SkFilterMode::kNearest, SkFilterMode::kLinear}) {
            SkPaint paint;

            // Draw single row image with mirror tiling in x and clamped in y.
            paint.setShader(imgx->makeShader(SkTileMode::kMirror,
                                             SkTileMode::kClamp,
                                             SkSamplingOptions{fm}));
            c->save();
            c->translate(imgx->width(), 0);
            if (offset) {
                c->translate(0.5, 0);
            }
            c->drawRect(SkRect::MakeXYWH(-imgx->width(), 0, 3*imgx->width(), 5), paint);
            c->restore();

            // Draw single column image with mirror tiling in y and clamped in x.
            paint.setShader(imgy->makeShader(SkTileMode::kClamp,
                                             SkTileMode::kMirror,
                                             SkSamplingOptions{fm}));
            c->save();
            c->translate(3*imgx->width() + 3, imgy->height());
            if (offset) {
                c->translate(0, 0.5);
            }
            c->drawRect(SkRect::MakeXYWH(0, -imgy->height(), 5, 3*imgy->height()), paint);
            c->restore();

            c->translate(0, 3*imgy->height() + 3);
        }
    }

    canvas->scale(8, 8);
    canvas->drawImage(surf->makeImageSnapshot(), 0, 0);
    return skiagm::DrawResult::kOk;
}
