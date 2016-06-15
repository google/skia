/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include <vector>

#include "gm.h"
#include "GrContext.h"
#include "Resources.h"
#include "SkImage.h"

#if SK_SUPPORT_GPU

// Helper function that uploads the given SkImage using MakdeFromDeferredTextureImageData and then
// draws the uploaded version at the specified coordinates.
static bool DrawDeferredTextureImageData(GrContext* context, SkCanvas* canvas, SkImage* image,
                                         SkImage::DeferredTextureImageUsageParams* params,
                                         SkScalar x, SkScalar y) {
    SkAutoTUnref<GrContextThreadSafeProxy> proxy(context->threadSafeProxy());
    size_t deferredSize =  image->getDeferredTextureImageData(*proxy, params, 1, nullptr);
    if (deferredSize == 0) {
        SkDebugf("\nCould not create DeferredTextureImageData.\n");
        return false;
    }

    std::vector<uint8_t> memory;
    memory.resize(deferredSize);
    image->getDeferredTextureImageData(*proxy, params, 1, memory.data());
    sk_sp<SkImage> uploadedImage =
        SkImage::MakeFromDeferredTextureImageData(context, memory.data(), SkBudgeted::kNo);
    canvas->drawImage(uploadedImage, x, y);

  return true;
}

DEF_SIMPLE_GM(deferred_texture_image_data, canvas, 60, 10) {
    GrContext* context = canvas->getGrContext();
    if (!context) {
        skiagm::GM::DrawGpuOnlyMessage(canvas);
        return;
    }

    sk_sp<SkImage> encodedImage = GetResourceAsImage("randPixels.png");
    if (!encodedImage) {
        SkDebugf("\nCould not load resource.\n");
        return;
    }

    SkBitmap bitmap;
    if (!GetResourceAsBitmap("randPixels.png", &bitmap)) {
        SkDebugf("\nCould not decode resource.\n");
        return;
    }

    sk_sp<SkImage> decodedImage = SkImage::MakeFromBitmap(bitmap);

    // Draw both encoded and decoded image via deferredTextureImageData.
    SkImage::DeferredTextureImageUsageParams params;
    DrawDeferredTextureImageData(context, canvas, encodedImage.get(), &params, 0, 0);
    DrawDeferredTextureImageData(context, canvas, decodedImage.get(), &params, 10, 0);

    // Draw 50% scaled versions of the encoded and decoded images at medium quality.
    SkImage::DeferredTextureImageUsageParams mediumScaledParams;
    mediumScaledParams.fPreScaleMipLevel = 1;
    mediumScaledParams.fQuality = kMedium_SkFilterQuality;

    DrawDeferredTextureImageData(context, canvas, encodedImage.get(), &mediumScaledParams, 20, 0);
    DrawDeferredTextureImageData(context, canvas, decodedImage.get(), &mediumScaledParams, 30, 0);

    // Draw 50% scaled versions of the encoded and decoded images at none quality.
    SkImage::DeferredTextureImageUsageParams noneScaledParams;
    noneScaledParams.fPreScaleMipLevel = 1;
    noneScaledParams.fQuality = kNone_SkFilterQuality;

    DrawDeferredTextureImageData(context, canvas, encodedImage.get(), &noneScaledParams, 40, 0);
    DrawDeferredTextureImageData(context, canvas, decodedImage.get(), &noneScaledParams, 50, 0);
}

#endif
