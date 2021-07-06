/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrTypes.h"
#include "src/core/SkDevice.h"
#include "src/core/SkSpecialImage.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "tests/Test.h"
#include "tools/gpu/GrContextFactory.h"

class SkColorSpace;

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


DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SpecialImage_GPUDevice, reporter, ctxInfo) {
    auto dContext = ctxInfo.directContext();

    static const int kWidth = 100;
    static const int kHeight = 90;

    SkImageInfo ii = SkImageInfo::MakeN32Premul(2*kWidth, 2*kHeight);

    auto device = dContext->priv().createDevice(SkBudgeted::kNo, ii, SkBackingFit::kExact,
                                                1, GrMipmapped::kNo, GrProtected::kNo,
                                                kBottomLeft_GrSurfaceOrigin, SkSurfaceProps(),
                                                skgpu::BaseDevice::kClear_InitContents);

    SkBitmap bm;
    SkAssertResult(bm.tryAllocN32Pixels(kWidth, kHeight));

    // Create a gpu-backed special image from a raster-backed SkBitmap
    sk_sp<SkSpecialImage> special = DeviceTestingAccess::MakeSpecial(device.get(), bm);
    SkASSERT(special->isTextureBacked());
    SkASSERT(kWidth == special->width());
    SkASSERT(kHeight == special->height());
    SkASSERT(bm.getGenerationID() == special->uniqueID());
    SkASSERT(SkIRect::MakeWH(kWidth, kHeight) == special->subset());

    // Create a gpu-backed special image from a raster-backed SkImage
    sk_sp<SkImage> image(bm.asImage());
    special = DeviceTestingAccess::MakeSpecial(device.get(), image.get());
    SkASSERT(special->isTextureBacked());
    SkASSERT(kWidth == special->width());
    SkASSERT(kHeight == special->height());
    // TODO: Hmmm, this is a bit unexpected
    SkASSERT(image->uniqueID() != special->uniqueID());
    SkASSERT(SkIRect::MakeWH(kWidth, kHeight) == special->subset());

    // Create a gpu-backed special image from a gpu-backed SkImage
    image = image->makeTextureImage(dContext);
    special = DeviceTestingAccess::MakeSpecial(device.get(), image.get());
    SkASSERT(special->isTextureBacked());
    SkASSERT(kWidth == special->width());
    SkASSERT(kHeight == special->height());
    SkASSERT(image->uniqueID() == special->uniqueID());
    SkASSERT(SkIRect::MakeWH(kWidth, kHeight) == special->subset());

    // Snap the device as a gpu-backed special image
    special = DeviceTestingAccess::SnapSpecial(device.get());
    SkASSERT(special->isTextureBacked());
    SkASSERT(2*kWidth == special->width());
    SkASSERT(2*kHeight == special->height());
    SkASSERT(SkIRect::MakeWH(2*kWidth, 2*kHeight) == special->subset());
}
