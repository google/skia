/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <src/core/SkAutoPixmapStorage.h>
#include <src/core/SkConvertPixels.h>
#include <tools/Resources.h>
#include "gm/gm.h"
#include "include/gpu/GrContext.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSurface.h"

DEF_SIMPLE_GM(async_rescale_and_read, canvas, 400, 301) {
    auto image = GetResourceAsImage("images/yellow_rose.webp");
    if (!image) {
        SkDebugf("Could not load yellow_rose.webp");
        return;
    }
    auto surf = canvas->makeSurface(image->imageInfo().makeWH(image->width(), image->height()));
    if (!surf) {
        return;
    }
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);
    surf->getCanvas()->drawImage(image, 0, 0, &paint);
    SkAutoPixmapStorage pm;
    pm.alloc(canvas->imageInfo());
    struct Context {
        SkAutoPixmapStorage* fPixmap;
        bool fCalled;
    } context{&pm, false};
    auto callback = [](void* c, const void* data, size_t rowBytes) {
        auto context = reinterpret_cast<Context*>(c);
        context->fCalled = true;
        if (!data) {
            context->fPixmap->reset();
            return;
        }
        SkRectMemcpy(context->fPixmap->writable_addr(), context->fPixmap->rowBytes(), data, rowBytes, context->fPixmap->info().minRowBytes(), context->fPixmap->height());
    };
    int w = canvas->imageInfo().width();
    int h = canvas->imageInfo().height();
    surf->asynRescaleAndcReadPixels(canvas->imageInfo().colorType(), canvas->imageInfo().alphaType(), canvas->imageInfo().refColorSpace(), SkIRect::MakeWH(image->width(), image->height()), w, h, SkSurface::RescaleLinear::kNo, kLow_SkFilterQuality, callback, &context);
    while (!context.fCalled) {
        SkASSERT(surf->getCanvas()->getGrContext());
        surf->getCanvas()->getGrContext()->checkAsyncWorkCompletion();
    }
    SkBitmap bmp;
    bmp.installPixels(pm);
    canvas->drawBitmap(bmp, 0, 0, &paint);
}
