/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapDevice.h"
#include "SkDevice.h"
#include "SkSpecialImage.h"

#if SK_SUPPORT_GPU
#include "SkGpuDevice.h"
#endif

#include "Test.h"

class DeviceTestingAccess {
public:
    static sk_sp<SkSpecialImage> MakeSpecial(SkBaseDevice* dev, const SkBitmap& bm) {
        return dev->makeSpecial(bm);
    }

    static sk_sp<SkSpecialImage> MakeSpecial(SkBaseDevice* dev, SkImage* img) {
        return dev->makeSpecial(img);
    }

    static sk_sp<SkSpecialImage> SnapSpecial(SkBaseDevice* dev) {
        return dev->snapSpecial();
    }
};

// TODO: re-enable this when Raster methods are implemented
#if 0
DEF_TEST(SpecialImage_BitmapDevice, reporter) {
    static const int kWidth = 100;
    static const int kHeight = 90;

    SkImageInfo ii = SkImageInfo::MakeN32Premul(2*kWidth, 2*kHeight);

    sk_sp<SkBaseDevice> bmDev(SkBitmapDevice::Create(ii));

    SkBitmap bm;
    bm.tryAllocN32Pixels(kWidth, kHeight);

    // Create a raster-backed special image from a raster-backed SkBitmap
    sk_sp<SkSpecialImage> special = DeviceTestingAccess::MakeSpecial(bmDev.get(), bm);
    SkASSERT(!special->isTextureBacked());
    SkASSERT(kWidth == special->width());
    SkASSERT(kHeight == special->height());
    SkASSERT(bm.getGenerationID() == special->uniqueID());
    SkASSERT(SkIRect::MakeWH(kWidth, kHeight) == special->subset());

    // Create a raster-backed special image from a raster-backed SkImage
    sk_sp<SkImage> image(SkImage::MakeFromBitmap(bm));
    special = DeviceTestingAccess::MakeSpecial(bmDev.get(), image.get());
    SkASSERT(!special->isTextureBacked());
    SkASSERT(kWidth == special->width());
    SkASSERT(kHeight == special->height());
    SkASSERT(bm.getGenerationID() == special->uniqueID());
    SkASSERT(SkIRect::MakeWH(kWidth, kHeight) == special->subset());

    // Snap the device as a raster-backed special image
    special = DeviceTestingAccess::SnapSpecial(bmDev.get());
    SkASSERT(!special->isTextureBacked());
    SkASSERT(2*kWidth == special->width());
    SkASSERT(2*kHeight == special->height());
    SkASSERT(SkIRect::MakeWH(2*kWidth, 2*kHeight) == special->subset());
}
#endif


#if SK_SUPPORT_GPU

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SpecialImage_GPUDevice, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    static const int kWidth = 100;
    static const int kHeight = 90;

    SkImageInfo ii = SkImageInfo::MakeN32Premul(2*kWidth, 2*kHeight);

    sk_sp<SkBaseDevice> gpuDev(SkGpuDevice::Make(context, SkBudgeted::kNo, ii,
                                                 0, kBottomLeft_GrSurfaceOrigin, nullptr,
                                                 SkGpuDevice::kClear_InitContents));

    SkBitmap bm;
    SkAssertResult(bm.tryAllocN32Pixels(kWidth, kHeight));

    // Create a gpu-backed special image from a raster-backed SkBitmap
    sk_sp<SkSpecialImage> special = DeviceTestingAccess::MakeSpecial(gpuDev.get(), bm);
    SkASSERT(special->isTextureBacked());
    SkASSERT(kWidth == special->width());
    SkASSERT(kHeight == special->height());
    SkASSERT(bm.getGenerationID() == special->uniqueID());
    SkASSERT(SkIRect::MakeWH(kWidth, kHeight) == special->subset());

    // Create a gpu-backed special image from a raster-backed SkImage
    sk_sp<SkImage> image(SkImage::MakeFromBitmap(bm));
    special = DeviceTestingAccess::MakeSpecial(gpuDev.get(), image.get());
    SkASSERT(special->isTextureBacked());
    SkASSERT(kWidth == special->width());
    SkASSERT(kHeight == special->height());
    // TODO: Hmmm, this is a bit unexpected
    SkASSERT(image->uniqueID() != special->uniqueID());
    SkASSERT(SkIRect::MakeWH(kWidth, kHeight) == special->subset());

    // Create a gpu-backed special image from a gpu-backed SkImage
    SkColorSpace* legacyColorSpace = nullptr;
    image = image->makeTextureImage(context, legacyColorSpace);
    special = DeviceTestingAccess::MakeSpecial(gpuDev.get(), image.get());
    SkASSERT(special->isTextureBacked());
    SkASSERT(kWidth == special->width());
    SkASSERT(kHeight == special->height());
    SkASSERT(image->uniqueID() == special->uniqueID());
    SkASSERT(SkIRect::MakeWH(kWidth, kHeight) == special->subset());

    // Snap the device as a gpu-backed special image
    special = DeviceTestingAccess::SnapSpecial(gpuDev.get());
    SkASSERT(special->isTextureBacked());
    SkASSERT(2*kWidth == special->width());
    SkASSERT(2*kHeight == special->height());
    SkASSERT(SkIRect::MakeWH(2*kWidth, 2*kHeight) == special->subset());
}

#endif
