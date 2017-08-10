/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include <vector>

#include "gm.h"
#include "SkImage.h"
#include "SkMipMap.h"
#include "Resources.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"

// Helper function that uploads the given SkImage using MakeFromDeferredTextureImageData and then
// draws the uploaded version at the specified coordinates.
static void DrawDeferredTextureImageData(SkCanvas* canvas,
                                         const char* resourceName,
                                         SkImage::DeferredTextureImageUsageParams* params,
                                         SkColorType dstColorType) {
    GrContext* context = canvas->getGrContext();
    if (!context) {
        skiagm::GM::DrawGpuOnlyMessage(canvas);
        return;
    }
    sk_sp<GrContextThreadSafeProxy> proxy(context->threadSafeProxy());



    sk_sp<SkImage> encodedImage = GetResourceAsImage(resourceName);
    if (!encodedImage) {
        SkDebugf("\nCould not load resource.\n");
        return;
    }

    size_t requiredMemoryInBytes = encodedImage->getDeferredTextureImageData(
        *proxy, params, 1, nullptr, canvas->imageInfo().colorSpace(), dstColorType);
    if (requiredMemoryInBytes == 0) {
        SkDebugf("\nCould not create DeferredTextureImageData.\n");
        return;
    }

    std::vector<uint8_t> memory;
    memory.resize(requiredMemoryInBytes);
    encodedImage->getDeferredTextureImageData(
        *proxy, params, 1, memory.data(), canvas->imageInfo().colorSpace(), dstColorType);
    sk_sp<SkImage> uploadedEncodedImage = SkImage::MakeFromDeferredTextureImageData(
        context, memory.data(), SkBudgeted::kNo);

    canvas->drawImage(uploadedEncodedImage, 10, 10);



    SkBitmap bitmap;
    if (!GetResourceAsBitmap(resourceName, &bitmap)) {
        SkDebugf("\nCould not decode resource.\n");
        return;
    }
    sk_sp<SkImage> decodedImage = SkImage::MakeFromBitmap(bitmap);

    requiredMemoryInBytes = decodedImage->getDeferredTextureImageData(
        *proxy, params, 1, nullptr, canvas->imageInfo().colorSpace(), dstColorType);
    if (requiredMemoryInBytes == 0) {
        SkDebugf("\nCould not create DeferredTextureImageData.\n");
        return;
    }

    memory.resize(requiredMemoryInBytes);
    decodedImage->getDeferredTextureImageData(
        *proxy, params, 1, memory.data(), canvas->imageInfo().colorSpace(), dstColorType);
    sk_sp<SkImage> uploadedDecodedImage = SkImage::MakeFromDeferredTextureImageData(
        context, memory.data(), SkBudgeted::kNo);

    canvas->drawImage(uploadedDecodedImage, encodedImage->width() + 20, 10);
}

static void DrawDeferredTextureImageMipMapTree(SkCanvas* canvas, SkImage* image,
                                               SkImage::DeferredTextureImageUsageParams* params,
                                               SkColorType dstColorType) {
    GrContext* context = canvas->getGrContext();
    if (!context) {
        skiagm::GM::DrawGpuOnlyMessage(canvas);
        return;
    }
    sk_sp<GrContextThreadSafeProxy> proxy(context->threadSafeProxy());

    SkPaint paint;
    paint.setFilterQuality(params->fQuality);

    int mipLevelCount = SkMipMap::ComputeLevelCount(image->width(), image->height());
    size_t requiredMemoryInBytes = image->getDeferredTextureImageData(
        *proxy, params, 1, nullptr, canvas->imageInfo().colorSpace(), dstColorType);
    if (requiredMemoryInBytes == 0) {
        SkDebugf("\nCould not create DeferredTextureImageData.\n");
        return;
    }

    std::vector<uint8_t> memory;
    memory.resize(requiredMemoryInBytes);
    image->getDeferredTextureImageData(
        *proxy, params, 1, memory.data(), canvas->imageInfo().colorSpace(), dstColorType);
    sk_sp<SkImage> uploadedImage = SkImage::MakeFromDeferredTextureImageData(
        context, memory.data(), SkBudgeted::kNo);

    // draw a column using deferred texture images
    SkScalar offsetHeight = 10.f;
    // handle base mipmap level
    canvas->save();
    canvas->translate(10.f, offsetHeight);
    canvas->drawImage(uploadedImage, 0, 0, &paint);
    canvas->restore();
    offsetHeight += image->height() + 10;
    // handle generated mipmap levels
    for (int i = 0; i < mipLevelCount; i++) {
        SkISize mipSize = SkMipMap::ComputeLevelSize(image->width(), image->height(), i);
        canvas->save();
        canvas->translate(10.f, offsetHeight);
        canvas->scale(mipSize.width() / static_cast<float>(image->width()),
                      mipSize.height() / static_cast<float>(image->height()));
        canvas->drawImage(uploadedImage, 0, 0, &paint);
        canvas->restore();
        offsetHeight += mipSize.height() + 10;
    }

    // draw a column using SkImage
    offsetHeight = 10;
    // handle base mipmap level
    canvas->save();
    canvas->translate(image->width() + 20.f, offsetHeight);
    canvas->drawImage(image, 0, 0, &paint);
    canvas->restore();
    offsetHeight += image->height() + 10;
    // handle generated mipmap levels
    for (int i = 0; i < mipLevelCount; i++) {
        SkISize mipSize = SkMipMap::ComputeLevelSize(image->width(), image->height(), i);
        canvas->save();
        canvas->translate(image->width() + 20.f, offsetHeight);
        canvas->scale(mipSize.width() / static_cast<float>(image->width()),
                      mipSize.height() / static_cast<float>(image->height()));
        canvas->drawImage(image, 0, 0, &paint);
        canvas->restore();
        offsetHeight += mipSize.height() + 10;
    }
}

DEF_SIMPLE_GM(deferred_texture_image_none, canvas, 512 + 512 + 30, 512 + 20) {
    auto params = SkImage::DeferredTextureImageUsageParams(SkMatrix::MakeScale(1.f, 1.f),
                                                           kNone_SkFilterQuality, 0);
    DrawDeferredTextureImageData(canvas, "mandrill_512.png", &params, kN32_SkColorType);
}

DEF_SIMPLE_GM(deferred_texture_image_low, canvas, 512 + 512 + 30, 512 + 20) {
    auto params = SkImage::DeferredTextureImageUsageParams(SkMatrix::MakeScale(1.f, 1.f),
                                                           kLow_SkFilterQuality, 0);
    DrawDeferredTextureImageData(canvas, "mandrill_512.png", &params, kN32_SkColorType);
}

DEF_SIMPLE_GM(deferred_texture_image_low_dithered, canvas, 180 + 180 + 30, 180 + 20) {
    auto params = SkImage::DeferredTextureImageUsageParams(SkMatrix::MakeScale(0.25f, 0.25f),
                                                           kLow_SkFilterQuality, 0);
    DrawDeferredTextureImageData(canvas, "dog.jpg", &params, kARGB_4444_SkColorType);
}

DEF_SIMPLE_GM(deferred_texture_image_medium_encoded, canvas, 512 + 512 + 30, 1110) {
    sk_sp<SkImage> encodedImage = GetResourceAsImage("mandrill_512.png");
    if (!encodedImage) {
        SkDebugf("\nCould not load resource.\n");
        return;
    }

    auto params = SkImage::DeferredTextureImageUsageParams(SkMatrix::MakeScale(0.25f, 0.25f),
                                                           kMedium_SkFilterQuality, 0);
    DrawDeferredTextureImageMipMapTree(canvas, encodedImage.get(), &params, kN32_SkColorType);
}

DEF_SIMPLE_GM(deferred_texture_image_medium_decoded, canvas, 512 + 512 + 30, 1110) {
    SkBitmap bitmap;
    if (!GetResourceAsBitmap("mandrill_512.png", &bitmap)) {
        SkDebugf("\nCould not decode resource.\n");
        return;
    }
    sk_sp<SkImage> decodedImage = SkImage::MakeFromBitmap(bitmap);

    auto params = SkImage::DeferredTextureImageUsageParams(SkMatrix::MakeScale(0.25f, 0.25f),
                                                           kMedium_SkFilterQuality, 0);
    DrawDeferredTextureImageMipMapTree(canvas, decodedImage.get(), &params, kN32_SkColorType);
}

DEF_SIMPLE_GM(deferred_texture_image_high, canvas, 512 + 512 + 30, 512 + 20) {
    auto params = SkImage::DeferredTextureImageUsageParams(SkMatrix::MakeScale(1.f, 1.f),
                                                           kHigh_SkFilterQuality, 0);
    DrawDeferredTextureImageData(canvas, "mandrill_512.png", &params, kN32_SkColorType);
}

DEF_SIMPLE_GM(deferred_texture_image_medium_encoded_indexed, canvas, 128 + 128 + 30, 340) {
    sk_sp<SkImage> encodedImage = GetResourceAsImage("color_wheel.gif");
    if (!encodedImage) {
        SkDebugf("\nCould not load resource.\n");
        return;
    }

    auto params = SkImage::DeferredTextureImageUsageParams(SkMatrix::MakeScale(0.25f, 0.25f),
                                                           kMedium_SkFilterQuality, 0);
    DrawDeferredTextureImageMipMapTree(canvas, encodedImage.get(), &params, kN32_SkColorType);
}

DEF_SIMPLE_GM(deferred_texture_image_medium_decoded_indexed, canvas, 128 + 128 + 30, 340) {
    SkBitmap bitmap;
    if (!GetResourceAsBitmap("color_wheel.gif", &bitmap)) {
        SkDebugf("\nCould not decode resource.\n");
        return;
    }
    sk_sp<SkImage> decodedImage = SkImage::MakeFromBitmap(bitmap);

    auto params = SkImage::DeferredTextureImageUsageParams(SkMatrix::MakeScale(0.25f, 0.25f),
                                                           kMedium_SkFilterQuality, 0);
    DrawDeferredTextureImageMipMapTree(canvas, decodedImage.get(), &params, kN32_SkColorType);
}

#endif
