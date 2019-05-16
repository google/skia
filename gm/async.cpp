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

static sk_sp<SkImage> do_read_and_scale(SkImage* image, const SkImageInfo& ii, SkSurface::RescaleLinear rescaleLiner, SkFilterQuality quality, std::function<sk_sp<SkSurface>(const SkImageInfo&)> makeSurface) {
    SkBitmap bmp;
    bmp.allocPixels(ii);
    // Turn the image into a surface in order to call the read and rescale API
    auto surf = makeSurface(image->imageInfo().makeWH(image->width(), image->height()));
    if (!surf) {
        return nullptr;
    }
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);
    surf->getCanvas()->drawImage(image, 0, 0, &paint);
    struct Context {
        SkPixmap fPixmap;
        bool fCalled = false;
    } context;
    SkAssertResult(bmp.peekPixels(&context.fPixmap));
    auto callback = [](void* c, const void* data, size_t rowBytes) {
      auto context = reinterpret_cast<Context*>(c);
      context->fCalled = true;
      if (!data) {
          context->fPixmap.reset();
          return;
      }
      SkRectMemcpy(context->fPixmap.writable_addr(), context->fPixmap.rowBytes(), data, rowBytes, context->fPixmap.info().minRowBytes(), context->fPixmap.height());
    };
    surf->asyncRescaleAndReadPixels(ii, SkIRect::MakeWH(image->width(), image->height()), rescaleLiner, quality, callback, &context);
    while (!context.fCalled) {
        // Only GPU should actually be asynchronous.
        SkASSERT(surf->getCanvas()->getGrContext());
        surf->getCanvas()->getGrContext()->checkAsyncWorkCompletion();
    }
    return SkImage::MakeFromBitmap(bmp);
}

static void do_rescale_grid(SkCanvas* canvas, const char* imageFile, int newW, int newH) {
    auto image = GetResourceAsImage(imageFile);
    if (!image)  {
        return;
    }
    newW = canvas->imageInfo().width() / 3;
    newH = canvas->imageInfo().height();
    const auto ii = canvas->imageInfo().makeWH(newW, newH);
    auto makeSurface = [canvas] (const SkImageInfo& info) { return canvas->makeSurface(info); };

    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);
    for (auto quality : {kNone_SkFilterQuality, kLow_SkFilterQuality, kHigh_SkFilterQuality}) {
        auto rescaled = do_read_and_scale(image.get(), ii, SkSurface::RescaleLinear::kNo, quality, makeSurface);
        canvas->drawImage(rescaled, 0, 0, &paint);
        canvas->translate(rescaled->width(), 0);
    }
}

DEF_SIMPLE_GM(async_rescale_and_read, canvas, 400, 301) {
    do_rescale_grid(canvas, "images/yellow_rose.webp", 400, 301);
}
