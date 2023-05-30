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

/**
 * Tests drawing images are half pixel offsets in device space with nearest filtering to show how
 * rasterization and image sample snapping at boundary points interact. Both drawImage and drawRect
 * with an image shader are tested. Scale factors 1 and -1 are tested. The images are all two pixels
 * wide or tall so we either get both values once each or one value repeated twice.
 */
DEF_SIMPLE_GM_CAN_FAIL(nearest_half_pixel_image, canvas, errorMsg, 264, 235) {
    // We don't run this test on the GPU because we're at the driver/hw's mercy for how this
    // is handled.
    if (canvas->recordingContext() || (canvas->getSurface() && canvas->getSurface()->recorder())) {
        *errorMsg = "Test is only relevant to CPU backend";
        return skiagm::DrawResult::kSkip;
    }

    // We make 2x1 and 1x2 images for each color type.
    struct Images {
        sk_sp<SkImage> imageX;
        sk_sp<SkImage> imageY;
    };

    Images images[2];
    uint32_t colors[] {0xFFFF0000, 0xFF0000FF};
    SkPixmap cpmx(SkImageInfo::Make({2, 1},
                                    kRGBA_8888_SkColorType,
                                    kPremul_SkAlphaType),
                  colors,
                  sizeof(colors));
    SkPixmap cpmy(SkImageInfo::Make({1, 2},
                                    kRGBA_8888_SkColorType,
                                    kPremul_SkAlphaType),
                  colors,
                  sizeof(colors[0]));
    images[0] = {SkImages::RasterFromPixmapCopy(cpmx), SkImages::RasterFromPixmapCopy(cpmy)};

    uint8_t alphas[] {0xFF, 0xAA};
    SkPixmap apmx(SkImageInfo::Make({2, 1},
                                    kAlpha_8_SkColorType,
                                    kPremul_SkAlphaType),
                  alphas,
                  sizeof(alphas));
    SkPixmap apmy(SkImageInfo::Make({1, 2},
                                    kAlpha_8_SkColorType,
                                    kPremul_SkAlphaType),
                  alphas,
                  sizeof(alphas[0]));
    images[1] = {SkImages::RasterFromPixmapCopy(apmx), SkImages::RasterFromPixmapCopy(apmy)};

    // We draw offscreen and then zoom that up to make the result clear.
    auto surf = canvas->makeSurface(canvas->imageInfo().makeWH(80, 80));
    if (!surf) {
        *errorMsg = "Test only works with SkSurface backed canvases";
        return skiagm::DrawResult::kSkip;
    }
    auto* c = surf->getCanvas();
    c->clear(SK_ColorWHITE);

    // We scale up in the direction not being tested, the one with image dimension of 1, to make the
    // result more easily visible.
    static const float kOffAxisScale = 4;

    auto draw = [&](sk_sp<SkImage> image, bool shader, bool doX, bool mirror, uint8_t alpha) {
        c->save();
        SkPaint paint;
        paint.setAlpha(alpha);
        if (shader) {
            paint.setShader(image->makeShader(SkFilterMode::kNearest));
        }
        if (doX) {
            c->scale(mirror ? -1 : 1, kOffAxisScale);
            c->translate(mirror ? -2.5 : 0.5, 0);
        } else {
            c->scale(kOffAxisScale, mirror ? -1 : 1);
            c->translate(0, mirror ? -2.5 : 0.5);
        }

        if (shader) {
            c->drawRect(SkRect::Make(image->dimensions()), paint);
        } else {
            c->drawImage(image, 0, 0, SkFilterMode::kNearest, &paint);
        }
        c->restore();
    };

    for (bool shader : {false, true})
    for (uint8_t alpha : {0xFF , 0x70}) {
        c->save();
        for (const auto& i : images)
        for (auto mirror : {false, true}) {
            draw(i.imageX, shader, /*doX=*/true, mirror, alpha);
            c->save();
            c->translate(4, 0);
            draw(i.imageY, shader, /*doX=*/false, mirror, alpha);
            c->restore();
            c->translate(0, kOffAxisScale*2);
        }
        c->restore();
        c->translate(kOffAxisScale*2, 0);
    }
    canvas->scale(8, 8);
    canvas->drawImage(surf->makeImageSnapshot(), 0, 0);

    return skiagm::DrawResult::kOk;
}
